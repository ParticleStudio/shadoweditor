FROM debian:latest as builder
RUN apt update -y
RUN apt install -y apt-transport-https lsb-release ca-certificates
RUN apt install -y build-essential clang unzip
# RUN pip install conan
ENV project_path /data/shadoweditor/
COPY / ${project_path}
WORKDIR ${project_path}
RUN xmake f --root -k shared -y -p linux -a x64 -m release
RUN xmake --root -w server

FROM debian:latest as runner
WORKDIR /opt/server/
COPY --from=builder /data/shadoweditor/build/linux/x64/release/* ./
VOLUME ["/opt/server/conf/","/opt/server/log/"]
CMD ["/opt/server/server /opt/server/conf/world"]
