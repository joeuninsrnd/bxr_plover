package destination;

import com.rabbitmq.client.AMQP;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.DeliverCallback;

public class Receive {

	private static final String RPC_QUEUE_NAME = "rpc_queue";

	public static void main(String[] argv) throws Exception {
		DBcon dbcon = new DBcon();
		ConnectionFactory factory = new ConnectionFactory();
		factory.setHost("localhost");

		try (Connection connection = factory.newConnection(); Channel channel = connection.createChannel()) {
			channel.exchangeDeclare(RPC_QUEUE_NAME, "fanout");
			String queueName = channel.queueDeclare(RPC_QUEUE_NAME, false, false, false, null).getQueue();
			channel.queueBind(queueName, RPC_QUEUE_NAME, "sb");
			channel.queuePurge(RPC_QUEUE_NAME);

			channel.basicQos(1);

			System.out.println(" [x] Awaiting RPC requests");

			Object monitor = new Object();
			DeliverCallback deliverCallback = (consumerTag, delivery) -> {
				AMQP.BasicProperties replyProps = new AMQP.BasicProperties.Builder()
						.correlationId(delivery.getProperties().getCorrelationId()).build();

				String message = "";

				try {
					message = new String(delivery.getBody(), "UTF-8");

					System.out.println(" [.] (" + message + ")");
					dbcon.createStatement(message);
				} catch (RuntimeException e) {
					System.out.println(" [.] " + e.toString());
				} finally {
					channel.basicPublish("", delivery.getProperties().getReplyTo(), replyProps,
							message.getBytes("UTF-8"));
					channel.basicAck(delivery.getEnvelope().getDeliveryTag(), false);
					// RabbitMq consumer worker thread notifies the RPC server owner thread
					synchronized (monitor) {
						monitor.notify();
					}
				}
			};

			channel.basicConsume(RPC_QUEUE_NAME, false, deliverCallback, (consumerTag -> {
			}));
			// Wait and be prepared to consume the message from RPC client.
			while (true) {
				synchronized (monitor) {
					try {
						monitor.wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	}
}
