# 将原始图片转换成需要的大小，并将其保存
import os
import tensorflow as tf
from PIL import Image

# 原始图片的存储位置
originPicturePath = 'C:/Users/Leonardo/OneDrive/Desktop/target/'

# 生成图片的存储位置
genPicture = './data/gen_picture'

# 需要的识别类型
classes = {'cabbage', 'carrot', 'nori', 'potato'}

# 样本总数
num_samples = 120


# 制作TFRecords数据
def create_record():
    writer = tf.compat.v1.python_io.TFRecordWriter("dishes_train.tfrecords")
    subDirs = os.listdir(originPicturePath)
    for subDir in subDirs:
        subDirPath = os.path.join(originPicturePath, subDir)
        # os.listdir() 方法用于返回指定的文件夹包含的文件或文件夹的名字的列表。
        index = 0
        for imgName in os.listdir(subDirPath):
            index += 1
            imgPath = os.path.join(subDirPath, imgName)
            img = Image.open(imgPath)
            img = img.resize((64, 64))  # 设置需要转换的图片大小
            img_raw = img.tobytes()  # 将图片转化为原生bytes
            print(index, img_raw)
            example = tf.train.Example(
                features=tf.train.Features(feature={
                    "label": tf.train.Feature(int64_list=tf.train.Int64List(value=[index])),
                    'img_raw': tf.train.Feature(bytes_list=tf.train.BytesList(value=[img_raw]))
                }))
            writer.write(example.SerializeToString())
    writer.close()


def read_and_decode(filename):
    # 创建文件队列,不限读取的数量
    filename_queue = tf.compat.v1.train.string_input_producer([filename])
    # create a reader from file queue
    reader = tf.TFRecordReader()
    # reader从文件队列中读入一个序列化的样本
    _, serialized_example = reader.read(filename_queue)
    # get feature from serialized example
    # 解析符号化的样本
    features = tf.parse_single_example(
        serialized_example,
        features={
            'label': tf.FixedLenFeature([], tf.int64),
            'img_raw': tf.FixedLenFeature([], tf.string)
        })
    label = features['label']
    img = features['img_raw']
    img = tf.decode_raw(img, tf.uint8)
    img = tf.reshape(img, [64, 64, 3])
    # img = tf.cast(img, tf.float32) * (1. / 255) - 0.5
    label = tf.cast(label, tf.int32)
    return img, label


if __name__ == '__main__':
    create_record()
    batch = read_and_decode('dishes_train.tfrecords')
    init_op = tf.group(tf.global_variables_initializer(), tf.local_variables_initializer())

    with tf.Session() as sess:  # 开始一个会话
        sess.run(init_op)
        coord = tf.train.Coordinator()
        threads = tf.train.start_queue_runners(coord=coord)

        for i in range(num_samples):
            example, lab = sess.run(batch)  # 在会话中取出image和label
            img = Image.fromarray(example, 'RGB')  # 这里Image是之前提到的
            img.save(genPicture + '/' + str(i) + 'samples' + str(lab) + '.jpg')  # 存下图片;注意cwd后边加上‘/’
            print(example, lab)
        coord.request_stop()
        coord.join(threads)
        sess.close()
