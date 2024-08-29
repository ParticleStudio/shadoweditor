FROM fedora:latest as project_build
RUN dnf install -y gcc-c++ clang unzip conan xmake
ENV project_path /data/shadoweditor/
COPY / ${project_path}
WORKDIR ${project_path}
RUN xmake f --root -k shared -y -p linux -a x64 -m release
RUN xmake --root -w server

FROM ubuntu:latest
RUN apt-get install quickjs
COPY --from=project_build /data/shadoweditor/build/linux/x64/release/* /opt/server/
CMD ["/opt/server/server /opt/server/conf/world"]
