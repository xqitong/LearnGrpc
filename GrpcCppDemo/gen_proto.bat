@echo off
chcp 65001


rem 定义需要处理的 .proto 文件名（用空格分隔）
set PROTO_NO_GRPC_FILES=Enums.proto
set PROTO_GRPC_FILES=Messages.proto


set PROTOC_PATH=grpc\bin\protoc.exe
set GRPC_CPP_PLUGIN_PATH=grpc\bin\grpc_cpp_plugin.exe
set PROTO_FILE=*.proto
set PROTO_PATH=.\protos
set CPP_FOLDER=cpp
set CPP_OUT_PATH=%PROTO_PATH%\%CPP_FOLDER%

echo CPP_OUT_PATH: %CPP_OUT_PATH%


rem 判断 cpp 文件夹是否存在
if exist "%CPP_OUT_PATH%" (
    echo 文件夹 "%CPP_OUT_PATH%" 已存在，正在删除...
    rmdir /s /q "%CPP_OUT_PATH%"
    echo 文件夹 "%CPP_OUT_PATH%" 已删除。
)
rem 创建 cpp 文件夹
mkdir "%CPP_OUT_PATH%"

rem 遍历 PROTO_GRPC_FILES 中的每个文件
for %%f in (%PROTO_GRPC_FILES%) do (
    echo 正在处理文件: %%f
    "%PROTOC_PATH%" --proto_path="%PROTO_PATH%" "%%f" --cpp_out="%CPP_OUT_PATH%"
    "%PROTOC_PATH%" --proto_path="%PROTO_PATH%" "%%f" --grpc_out="%CPP_OUT_PATH%" --plugin=protoc-gen-grpc="%GRPC_CPP_PLUGIN_PATH%"
)
rem 遍历 PROTO_NO_GRPC_FILES 中的每个文件
for %%f in (%PROTO_NO_GRPC_FILES%) do (
    echo 正在处理文件: %%f
    "%PROTOC_PATH%" --proto_path="%PROTO_PATH%" "%%f" --cpp_out="%CPP_OUT_PATH%"
)

echo 所有 .proto 文件处理完成！

echo 拷贝到工程文件夹
xcopy /r /c /s /e /y "%CPP_OUT_PATH%\*" "GrpcServer\protos"
xcopy /r /c /s /e /y "%CPP_OUT_PATH%\*" "GrpcClient\protos"

pause