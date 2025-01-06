// GrpcServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <format>
#include "protos/Messages.grpc.pb.h"

constexpr char kServerCertPath[] = "D:\\Repos\\LearnGrpc\\GrpcCppDemo\\pems\\cert.pem";
constexpr char kServerKeyPath[] = "D:\\Repos\\LearnGrpc\\GrpcCppDemo\\pems\\key.pem";

std::string LoadStringFromFile(std::string path) {
    if (!std::filesystem::exists(path))
    {
        std::cout << "not exists " << path << std::endl;
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open " << path << std::endl;
        abort();
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    return sstr.str();
}

class EmployeeServiceImpl final :public EmployeeService::Service
{
public:
    virtual ::grpc::Status GetByNo(::grpc::ServerContext* context, const ::GetByNoRequest* request, ::EmployeeResponse* response)
    {
        std::cout << "GetByNo called \n";
        for (const auto& emp : employees_)
        {
            if (emp.no() == request->no())
            {
                std::cout << "Employee found:" << emp.DebugString() << "\n";
                response->mutable_employee()->CopyFrom(emp);
                return grpc::Status::OK;
            }
        }
        std::cout << "Employee not found:\n";
        return grpc::Status::CANCELLED;
    }
    virtual ::grpc::Status GetAll(::grpc::ServerContext* context, const ::GetAllRequest* request, ::grpc::ServerWriter< ::EmployeeResponse>* writer) {
        std::cout << "GetAll called \n";
        EmployeeResponse response;
        for (const auto& emp : employees_)
        {
            response.mutable_employee()->CopyFrom(emp);
            writer->Write(response);
        }
        return grpc::Status::OK;
    }
    virtual ::grpc::Status AddPhoto(::grpc::ServerContext* context, ::grpc::ServerReader< ::AddPhotoRequest>* reader, ::AddPhotoResponse* response) {
        
        auto rg = context->client_metadata().equal_range("no");
        //rg.first->second.data();
        std::cout << std::format("Employee:{}", rg.first->second.data()) << "\n";
        
        std::vector<uint8_t> img;
        AddPhotoRequest data;

        while (reader->Read(&data)) {
            std::cout << "Received " << data.data().size() << " bytes" << std::endl;
            img.insert(img.end(), data.data().begin(), data.data().end());
        }

        std::cout << "File Size: " << img.size() << std::endl;


        response->set_isok(true);
        std::cout << "AddPhoto called \n";
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Save(::grpc::ServerContext* context, const ::EmployeeRequest* request, ::EmployeeResponse* response) {
        std::cout << "Save called \n";
        return grpc::Status::CANCELLED;
    }
    virtual ::grpc::Status SaveAll(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::EmployeeResponse, ::EmployeeRequest>* stream) {
        
        EmployeeRequest request;
        while (stream->Read(&request))
        {
            const auto & emp = request.employee();
            {
                std::lock_guard<std::mutex> lock(mutex_);
                employees_.push_back(emp);
            }
            EmployeeResponse response;
            response.mutable_employee()->CopyFrom(emp);
            stream->Write(response);

        }
        std::cout << "Employees: " << employees_.size() << std::endl;
        for (const auto& employee : employees_) {
            std::cout << employee.ShortDebugString() << std::endl;
        }
        std::cout << "SaveAll called \n";
        return grpc::Status::OK;
    }
    virtual ::grpc::Status CreateToken(::grpc::ServerContext* context, const ::TokenRequest* request, ::TokenResponse* response) {
        return grpc::Status::CANCELLED;
    }
    EmployeeServiceImpl()
    {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        employees_ = {
            // 第一个 Employee
            [&now_seconds] {
                Employee emp;
                emp.set_id(1);
                emp.set_no(101);
                emp.set_firstname("John");
                emp.set_lastname("Doe");
                emp.mutable_monthsalary()->set_basic(5000);
                emp.mutable_monthsalary()->set_bonus(1000);
                emp.set_status(EmployeeStatus::NORMAL);
                emp.mutable_lastmodified()->set_seconds(now_seconds);
                return emp;
            }(),
                // 第二个 Employee
                [&now_seconds] {
                    Employee emp;
                    emp.set_id(2);
                    emp.set_no(102);
                    emp.set_firstname("Jane");
                    emp.set_lastname("Smith");
                    emp.mutable_monthsalary()->set_basic(1000);
                    emp.mutable_monthsalary()->set_bonus(3000);
                    emp.set_status(EmployeeStatus::RESIGNED);
                    emp.mutable_lastmodified()->set_seconds(now_seconds);
                    return emp;
                }(),
                    // 第三个 Employee
                    [&now_seconds] {
                        Employee emp;
                        emp.set_id(3);
                        emp.set_no(103);
                        emp.set_firstname("Alice");
                        emp.set_lastname("Johnson");
                        emp.mutable_monthsalary()->set_basic(10000);
                        emp.mutable_monthsalary()->set_bonus(300);
                        emp.set_status(EmployeeStatus::RETIRED);
                        emp.mutable_lastmodified()->set_seconds(now_seconds);
                        return emp;
                    }()
        };
    }
private:
    std::vector<Employee> employees_;
    std::mutex mutex_;
};
 constexpr int port = 5001;
int main()
{
    std::string server_address = std::format("localhost:{}", port);
    EmployeeServiceImpl service;
    grpc::ServerBuilder builder;
    // Load SSL credentials and build a SSL credential options
    grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair = {
        LoadStringFromFile(kServerKeyPath), LoadStringFromFile(kServerCertPath) };
    grpc::SslServerCredentialsOptions ssl_options;
    ssl_options.pem_key_cert_pairs.emplace_back(key_cert_pair);
    // Listen on the given address with SSL credentials
    builder.AddListeningPort(server_address,
        grpc::SslServerCredentials(ssl_options));
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}