# 建立神经网络
import tensorflow as tf


# 网络结构定义
# 输入参数：images，image batch、4D tensor、tf.float32、[batch_size, width, height, channels]
# 返回参数：logits, float、 [batch_size, n_classes]
def inference(images, batch_size, n_classes):
    # 构建一个简单的卷积神经网络，其中（卷积+池化层）x2，全连接层x2，最后一个softmax层做分类。

    # 卷积层1
    # 64个3x3的卷积核（3通道），padding=’SAME’，表示padding后卷积的图与原图尺寸一致，激活函数relu()
    # tf.variable_scope 可以让变量有相同的命名，包括tf.get_variable得到的变量，还有tf.Variable变量
    # 它返回的是一个用于定义创建variable(层)的op的上下文管理器。
    with tf.variable_scope('conv1') as scope:
        # tf.truncated_normal截断的产生正态分布的随机数，即随机数与均值的差值若大于两倍的标准差，则重新生成。
        # shape，生成张量的维度
        # mean，均值
        # stddev，标准差
        weights = tf.Variable(tf.truncated_normal(shape=[3, 3, 3, 64], stddev=1.0, dtype=tf.float32),
                              name='weights', dtype=tf.float32)

        # 偏置biases计算
        # shape表示生成张量的维度
        # 生成初始值为0.1的偏执biases
        biases = tf.Variable(tf.constant(value=0.1, dtype=tf.float32, shape=[64]),
                             name='biases', dtype=tf.float32)

        # 卷积层计算
        # 输入图片x和所用卷积核w
        # x是对输入的描述，是一个4阶张量：
        # 比如：[batch,5,5,3]
        # 第一阶给出一次喂入多少张图片也就是batch
        # 第二阶给出图片的行分辨率
        # 第三阶给出图片的列分辨率
        # 第四阶给出输入的通道数
        # w是对卷积核的描述，也是一个4阶张量：
        # 比如：[3,3,3,16]
        # 第一阶第二阶分别给出卷积行列分辨率
        # 第三阶是通道数
        # 第四阶是有多少个卷积核
        # strides卷积核滑动步长：[1,1,1,1]
        # 第二阶第三阶表示横向纵向滑动的步长
        # 第一和第四阶固定为1
        # 使用0填充，所以padding值为SAME
        conv = tf.nn.conv2d(images, weights, strides=[1, 1, 1, 1], padding='SAME')

        # 非线性激活
        # 对卷积后的conv1添加偏执，通过relu激活函数
        pre_activation = tf.nn.bias_add(conv, biases)

        conv1 = tf.nn.relu(pre_activation, name=scope.name)

    # 池化层1
    # 3x3最大池化，步长strides为2，池化后执行lrn()操作，局部响应归一化，对训练有利。
    # 最大池化层计算
    # x是对输入的描述，是一个四阶张量：
    # 比如：[batch,28,28,6]
    # 第一阶给出一次喂入多少张图片batch
    # 第二阶给出图片的行分辨率
    # 第三阶给出图片的列分辨率
    # 第四阶输入通道数
    # 池化核大小2*2的
    # 行列步长都是2
    # 使用SAME的padding
    with tf.variable_scope('pooling1_lrn') as scope:
        pool1 = tf.nn.max_pool(conv1, ksize=[1, 3, 3, 1], strides=[1, 2, 2, 1], padding='SAME', name='pooling1')

        # 局部响应归一化函数tf.nn.lrn
        norm1 = tf.nn.lrn(pool1, depth_radius=4, bias=1.0, alpha=0.001 / 9.0, beta=0.75, name='norm1')

    # 卷积层2
    # 16个3x3的卷积核（16通道），padding=’SAME’，表示padding后卷积的图与原图尺寸一致，激活函数relu()
    with tf.variable_scope('conv2') as scope:
        weights = tf.Variable(tf.truncated_normal(shape=[3, 3, 64, 16], stddev=0.1, dtype=tf.float32),
                              name='weights', dtype=tf.float32)

        biases = tf.Variable(tf.constant(value=0.1, dtype=tf.float32, shape=[16]),
                             name='biases', dtype=tf.float32)

        conv = tf.nn.conv2d(norm1, weights, strides=[1, 1, 1, 1], padding='SAME')

        pre_activation = tf.nn.bias_add(conv, biases)

        conv2 = tf.nn.relu(pre_activation, name='conv2')

    # 池化层2
    # 3x3最大池化，步长strides为2，池化后执行lrn()操作，
    # pool2 and norm2
    with tf.variable_scope('pooling2_lrn') as scope:
        norm2 = tf.nn.lrn(conv2, depth_radius=4, bias=1.0, alpha=0.001 / 9.0, beta=0.75, name='norm2')

        pool2 = tf.nn.max_pool(norm2, ksize=[1, 3, 3, 1], strides=[1, 1, 1, 1], padding='SAME', name='pooling2')

    # 全连接层3
    # 128个神经元，将之前pool层的输出reshape成一行，激活函数relu()
    with tf.variable_scope('local3') as scope:
        # 函数的作用是将tensor变换为参数shape的形式。 其中shape为一个列表形式，特殊的一点是列表中可以存在-1。
        # -1代表的含义是不用我们自己指定这一维的大小，函数会自动计算，但列表中只能存在一个-1。
        reshape = tf.reshape(pool2, shape=[batch_size, -1])

        # get_shape返回的是一个元组
        dim = reshape.get_shape()[1].value

        weights = tf.Variable(tf.truncated_normal(shape=[dim, 128], stddev=0.005, dtype=tf.float32),
                              name='weights', dtype=tf.float32)

        biases = tf.Variable(tf.constant(value=0.1, dtype=tf.float32, shape=[128]),
                             name='biases', dtype=tf.float32)

        local3 = tf.nn.relu(tf.matmul(reshape, weights) + biases, name=scope.name)

    # 全连接层4
    # 128个神经元，激活函数relu()
    with tf.variable_scope('local4') as scope:
        weights = tf.Variable(tf.truncated_normal(shape=[128, 128], stddev=0.005, dtype=tf.float32),
                              name='weights', dtype=tf.float32)

        biases = tf.Variable(tf.constant(value=0.1, dtype=tf.float32, shape=[128]),
                             name='biases', dtype=tf.float32)

        local4 = tf.nn.relu(tf.matmul(local3, weights) + biases, name='local4')

    # dropout层
    #    with tf.variable_scope('dropout') as scope:
    #        drop_out = tf.nn.dropout(local4, 0.8)

    # Softmax回归层
    # 将前面的FC层输出，做一个线性回归，计算出每一类的得分，在这里是2类，所以这个层输出的是两个得分。
    with tf.variable_scope('softmax_linear') as scope:
        weights = tf.Variable(tf.truncated_normal(shape=[128, n_classes], stddev=0.005, dtype=tf.float32),
                              name='softmax_linear', dtype=tf.float32)

        biases = tf.Variable(tf.constant(value=0.1, dtype=tf.float32, shape=[n_classes]),
                             name='biases', dtype=tf.float32)

        softmax_linear = tf.add(tf.matmul(local4, weights), biases, name='softmax_linear')

    return softmax_linear


# loss计算
# 传入参数：logits，网络计算输出值。labels，真实值，在这里是0或者1
# 返回参数：loss，损失值
def losses(logits, labels):
    with tf.variable_scope('loss') as scope:
        # 传入的logits为神经网络输出层的输出，shape为[batch_size，num_classes]，
        # 传入的label为一个一维的vector，长度等于batch_size，
        # 每一个值的取值区间必须是[0，num_classes)，其实每一个值就是代表了batch中对应样本的类别
        cross_entropy = tf.nn.sparse_softmax_cross_entropy_with_logits(logits=logits, labels=labels,
                                                                       name='xentropy_per_example')

        # tf.reduce_mean 函数用于计算张量tensor沿着指定的数轴（tensor的某一维度）上的的平均值，
        # 主要用作降维或者计算tensor（图像）的平均值。
        loss = tf.reduce_mean(cross_entropy, name='loss')

        # tf.summary.scalar用来显示标量信息
        # 一般在画loss,accuary时会用到这个函数。
        tf.summary.scalar(scope.name + '/loss', loss)

    return loss


# loss损失值优化
# 输入参数：loss。learning_rate，学习速率。
# 返回参数：train_op，训练op，这个参数要输入sess.run中让模型去训练。
def trainning(loss, learning_rate):
    with tf.name_scope('optimizer'):
        # tf.train.AdamOptimizer()函数是Adam优化算法：是一个寻找全局最优点的优化算法，引入了二次方梯度校正。
        # learning_rate：张量或浮点值。学习速率
        optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate)

        global_step = tf.Variable(0, name='global_step', trainable=False)

        # minimize() 实际上包含了两个步骤，即 compute_gradients 和 apply_gradients，
        # 前者用于计算梯度，后者用于使用计算得到的梯度来更新对应的variable
        train_op = optimizer.minimize(loss, global_step=global_step)

    return train_op


# 评价/准确率计算
# 输入参数：logits，网络计算值。labels，标签，也就是真实值，在这里是0或者1。
# 返回参数：accuracy，当前step的平均准确率，也就是在这些batch中多少张图片被正确分类了。
def evaluation(logits, labels):
    with tf.variable_scope('accuracy') as scope:
        # tf.nn.in_top_k用于计算预测的结果和实际结果的是否相等，并返回一个bool类型的张量
        correct = tf.nn.in_top_k(logits, labels, 1)

        # tf.cast()函数的作用是执行 tensorflow 中张量数据类型转换
        correct = tf.cast(correct, tf.float16)

        # tf.reduce_mean计算张量的各个维度的元素的量
        accuracy = tf.reduce_mean(correct)

        # tf.summary.scalar用来显示标量信息
        tf.summary.scalar(scope.name + '/accuracy', accuracy)

    return accuracy
