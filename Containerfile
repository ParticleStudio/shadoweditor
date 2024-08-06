FROM fedora:latest as project_build
RUN dnf install -y gcc-c++ clang unzip conan xmake
ENV project_path /data/shadoweditor/
COPY / ${project_path}
WORKDIR ${project_path}
RUN xmake f --root -k shared -y -p linux -a x64 -m release
RUN xmake --root -w server

FROM fedora:latest
RUN ls
COPY --from=project_build /data/shadoweditor/build/linux/x64/release/* /opt/server
CMD ["server 1"]
