from PIL import Image
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt
import forward
from batchdealing import get_files


# 获取一张图片
def get_one_image(train):
    # 输入参数：train,训练图片的路径
    # 返回参数：image，从训练图片中随机抽取一张图片
    n = len(train)
    ind = np.random.randint(0, n)
    img_dir = train[ind]  # 随机选择测试的图片

    img = Image.open(img_dir)

    # 显示图片，在jupyter notebook下当然也可以不用plt.show()
    plt.imshow(img)
    plt.show(img)
    imag = img.resize([64, 64])  # 由于图片在预处理阶段以及resize，因此该命令可略
    image = np.array(imag)
    return image


# 测试图片
def evaluate_one_image(image_array):
    with tf.Graph().as_default():
        BATCH_SIZE = 1
        N_CLASSES = 4

        image = tf.cast(image_array, tf.float32)

        # 线性缩放图像以具有零均值和单位范数。
        image = tf.image.per_image_standardization(image)
        image = tf.reshape(image, [1, 64, 64, 3])

        # 构建卷积神经网络
        logit = forward.inference(image, BATCH_SIZE, N_CLASSES)

        # softmax函数的作用就是归一化
        # 输入: 全连接层（往往是模型的最后一层）的值，一般代码中叫做logits。
        # 输出: 归一化的值，含义是属于该位置的概率，一般代码叫做probs。
        logit = tf.nn.softmax(logit)

        x = tf.placeholder(tf.float32, shape=[64, 64, 3])

        # you need to change the directories to yours. E:/python-run-env/train-test/Re_train/image_data/inputdata/
        logs_train_dir = 'E:/python-run-env/train-test/Re_train/image_data/inputdata/'

        # tf.train.Saver() 保存和加载模型
        saver = tf.train.Saver()

        with tf.Session() as sess:

            print("Reading checkpoints...")
            ckpt = tf.train.get_checkpoint_state(logs_train_dir)
            if ckpt and ckpt.model_checkpoint_path:
                global_step = ckpt.model_checkpoint_path.split('/')[-1].split('-')[-1]
                saver.restore(sess, ckpt.model_checkpoint_path)
                print('Loading success, global_step is %s' % global_step)
            else:
                print('No checkpoint file found')

            # feed_dict的作用是给使用placeholder创建出来的tensor赋值。
            # 其实，他的作用更加广泛：feed使用一个值临时替换一个op的输出结果。
            # 你可以提供feed数据作为run()调用的参数。
            # feed只在调用它的方法内有效,方法结束,feed就会消失。
            # 当我们构建完图后，需要在一个会话中启动图，启动的第一步是创建一个Session对象。
            # 为了取回（Fetch）操作的输出内容，可以在使用Session对象的run()调用执行图时，
            # 传入一些tensor，这些tensor会帮助你取回结果。
            prediction = sess.run(logit, feed_dict={x: image_array})

            #  取出prediction中元素最大值所对应的索引，也就是最大的可能
            max_index = np.argmax(prediction)

            if max_index == 0:
                print('This is a cabbage with possibility %.6f' % prediction[:, 0])
            elif max_index == 1:
                print('This is a carrot with possibility %.6f' % prediction[:, 1])
            elif max_index == 2:
                print('This is a nori with possibility %.6f' % prediction[:, 2])
            else:
                print('This is a potato with possibility %.6f' % prediction[:, 3])


if __name__ == '__main__':
    train_dir = 'E:/python-run-env/train-test/Re_train/image_data/inputdata/'
    train, train_label, val, val_label = get_files(train_dir, 0.3)
    img = get_one_image(val)  # 通过改变参数train or val，进而验证训练集或测试集
    evaluate_one_image(img)
