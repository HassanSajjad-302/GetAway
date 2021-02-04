
#ifndef PRACTISE_INFERIOR_HPP
#define PRACTISE_INFERIOR_HPP


#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "asio.hpp"
#include <type_traits>

/*template <typename T>
concept Master =
requires(T t) {
    { t.outputReceived(std::string()) } -> std::same_as<void>;
};*/


template<typename Master, const char*... execArgs>
class inferior {

    asio::io_context& io;
    decltype(asio::posix::basic_stream_descriptor(io)) desc;
    int readingPipeForInferior[2];
    int writingPipeForInferior[2];
    Master* master;
    std::vector<char> buffer;
    void outputReceivedFromInferior(const std::error_code &errorCode);

public:
    inferior(Master* master_, asio::io_context& io_);
    void sendInputToInferior(const std::string& str);
    void receiveOutputFromInferior();
};

template<typename Master, const char *... execArgs>
inferior<Master, execArgs...>::inferior(Master* master_, asio::io_context& io_):
        io{io_}, desc{io_}, master{master_} {
    buffer.resize(4096);
    int p = pipe(readingPipeForInferior);
    if(p !=0){
        std::cerr<<"Could Not Open The Pipes In inferior constructor"<<std::endl;
        exit(-1);
    }
    int q = pipe(writingPipeForInferior);
    if(q != 0){
        std::cerr<<"Could Not Open The Pipes In inferior constructor"<<std::endl;
        exit(-1);
    }
    desc.assign(writingPipeForInferior[0]);
    int pid = fork();
    if(pid == -1){
        std::cerr<<"Error In Forking In inferior constructor"<<std::endl;
        exit(-1);
    }else if(pid == 0){
        close(readingPipeForInferior[1]);
        close(writingPipeForInferior[0]);
        dup2(readingPipeForInferior[0], STDIN_FILENO);
        dup2(writingPipeForInferior[1], STDOUT_FILENO);
        execl(execArgs..., nullptr);
    }else{
        close(readingPipeForInferior[0]);
        close(writingPipeForInferior[1]);
    }
}

template<typename Master, const char *... execArgs>
void inferior<Master, execArgs...>::outputReceivedFromInferior(const std::error_code &errorCode) {

    std::string str;
    int readSize = read(writingPipeForInferior[0], &buffer[0], buffer.size());
    while(readSize >0){
        str += std::string(buffer.begin(), buffer.begin() + readSize);
        readSize = read(writingPipeForInferior[0], buffer.begin().base(), buffer.size());
    }
    master->outputReceived(std::move(str));
}

template<typename Master, const char *... execArgs>
void inferior<Master, execArgs...>::sendInputToInferior(const std::string& str) {
    write(readingPipeForInferior[1], str.data(), str.size());
}

template<typename Master, const char *... execArgs>
void inferior<Master, execArgs...>::receiveOutputFromInferior() {
    desc.template async_wait(asio::posix::descriptor_base::wait_type::wait_read,
                             [ptr = this](const std::error_code& errorCode){
                                 ptr->outputReceivedFromInferior(errorCode);
                             });
}

#endif //PRACTISE_INFERIOR_HPP
