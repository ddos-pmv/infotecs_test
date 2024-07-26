#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class SharedBuffer {
public:
    SharedBuffer() : data_ready(false) {}

    void produce(const std::string& input) {
        std::unique_lock<std::mutex> lock(mtx);
        buffer = input;
        data_ready = true;
        cv.notify_one();
    }

    std::string consume() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return data_ready; });
        data_ready = false;
        return buffer;
    }

private:
    std::string buffer;
    bool data_ready;
    std::mutex mtx;
    std::condition_variable cv;
};

bool is_digit_string(const std::string& str) {
    return str.size() <= 64 && std::all_of(str.begin(), str.end(), ::isdigit);
}

std::string process_input(const std::string& input) {
    std::string sorted_input = input;
    std::sort(sorted_input.begin(), sorted_input.end(), std::greater<char>());
    for (size_t i = 0; i < sorted_input.size(); ++i) {
        if ((sorted_input[i] - '0') % 2 == 0) {
            sorted_input.replace(i, 1, "KB");
            i++;
        }
    }
    return sorted_input;
}

void producer(SharedBuffer& sharedBuffer, std::mutex& io_mutex) {
    while (true) {
        std::string input;

        // Synchronize input
        {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "Enter a numeric string: ";
            std::cin >> input;
        }

        if (!is_digit_string(input)) {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "Invalid input. Please enter a numeric string with a maximum of 64 characters." << std::endl;
            continue;
        }

        std::string processed = process_input(input);
        sharedBuffer.produce(processed);
    }
}

void consumer(SharedBuffer& sharedBuffer, std::mutex& io_mutex) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    while (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    while (true) {
        std::string data = sharedBuffer.consume();

        int sum = 0;
        for (char c : data) {
            if (isdigit(c)) {
                sum += c - '0';
            }
        }


        send(sockfd, &sum, sizeof(sum), 0);
    }

    close(sockfd);
}

int main() {
    SharedBuffer sharedBuffer;
    std::mutex io_mutex;

    std::thread t1(producer, std::ref(sharedBuffer), std::ref(io_mutex));
    std::thread t2(consumer, std::ref(sharedBuffer), std::ref(io_mutex));

    t1.join();
    t2.join();

    return 0;
}
