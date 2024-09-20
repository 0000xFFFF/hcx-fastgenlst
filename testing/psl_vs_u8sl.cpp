#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>

std::mutex print_mutex;

size_t utf8_strlen(const std::string& str) {
    size_t length = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if      (c <= 0x7F)              { i += 1;    } // 1-byte character (ASCII)
        else if (c >= 0xC2 && c <= 0xDF) { i += 2;    } // 2-byte character
        else if (c >= 0xE0 && c <= 0xEF) { i += 3;    } // 3-byte character
        else if (c >= 0xF0 && c <= 0xF4) { i += 4;    } // 4-byte character
        else                             { continue;  } // Invalid UTF-8 sequence encountered at byte i
                                                        // -1 == 18446744073709551615
        ++length;
    }
    return length;
}

size_t get_python_strlen(const std::string& str, const size_t current_line) {
    
    std::string temp_file_name = "temp_input_" + std::to_string(current_line) + ".txt";
    std::ofstream temp_file(temp_file_name);
    if (!temp_file) {
        std::cerr << "Error: Could not create temporary file\n";
        return static_cast<size_t>(-1);
    }
    temp_file << str;
    temp_file.close();

    std::string command = "python3 -c \"with open('" + temp_file_name + "', 'r', encoding='utf-8', errors='ignore') as f: print(len(f.read()))\"";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return static_cast<size_t>(-1);
    
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) { result += buffer; }
    
    pclose(pipe);
    std::filesystem::remove(temp_file_name);
    return std::stoi(result);
}

size_t get_file_line_count(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error: Could not open file " << filename << '\n';
        return 0;
    }

    size_t line_count = 0;
    std::string line;
    while (std::getline(infile, line)) {
        ++line_count;
    }
    infile.close();
    return line_count;
}

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void enqueue(const std::function<void()>& task);
    void wait_for_completion();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(const std::function<void()>& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

void ThreadPool::wait_for_completion() {
    // No explicit need for this function in this case as workers join on destruction
}


/*
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    const std::string filename(argv[1]);

    int i = 0;
    int n = get_file_line_count(filename);

    // Open file from command-line argument
    std::ifstream infile(filename, std::ifstream::binary);
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << '\n';
        return 1;
    }


    std::string line;
    while (std::getline(infile, line)) {
        i++;
        std::cout << i << "/" << n << " -- '" << line << "' -- "; 
        std::cout.flush();
        size_t cpp_length = utf8_strlen(line);  // C++ string length using utf8_strlen
        std::cout << cpp_length << " ";
        std::cout.flush();
        size_t python_length = get_python_strlen(line);  // Python len() length
        std::cout << python_length << " " << std::endl;
        if (cpp_length != python_length) {
            std::cerr << i << "/" << n << " -- '" << line << "' -- " << cpp_length << " " << python_length << std::endl;
        }
    }

    infile.close();
    return 0;
}
*/

//*
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <num_threads>\n";
        return 1;
    }

    const std::string filename(argv[1]);

    size_t total_lines = get_file_line_count(filename);

    std::ifstream infile(filename, std::ifstream::binary);
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << '\n';
        return 1;
    }

    std::string line;
    size_t current_line = 0;

    size_t num_threads = std::stoi(argv[2]);
    ThreadPool pool(num_threads);
    while (std::getline(infile, line)) {
        ++current_line;
        pool.enqueue([line, current_line, total_lines] {
            size_t cpp_length = utf8_strlen(line);
            size_t python_length = get_python_strlen(line, current_line);
            if (cpp_length != python_length || current_line == 299) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << current_line << "/" << total_lines << " -- '" << line << "' -- " 
                          << cpp_length << " " << python_length << std::endl;
            }
        });
    }

    infile.close();
    pool.wait_for_completion();
    return 0;
}
/**/
