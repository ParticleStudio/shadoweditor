import tensorflow as tf

if __name__ == '__main__':
    print("tensorflow version: ", tf.__version__)  # 查看tensorflow版本
    print("tensorflow path", tf.__path__)  # 查看tensorflow安装路径

    isBuiltWithCuda = tf.test.is_built_with_cuda()  # 判断CUDA是否可以用
    print("is built with cuda: ", isBuiltWithCuda)  # 显示True表示CUDA可用

    isGpuAvailable = tf.test.is_gpu_available(
        cuda_only=False,
        min_cuda_compute_capability=None
    )  # 判断GPU是否可以用
    print("is gpu available: ", isGpuAvailable)  # 显示True表示GPU可用

    # 查看驱动名称
    if tf.test.gpu_device_name():
        print('Default GPU Device: {}'.format(tf.test.gpu_device_name()))
    else:
        print("Please install GPU version of TF")
