#include <arpa/inet.h>
#include <string.h>
#include "nanomsg/nn.h"
#include "ylog.h"
#include "command_put.h"
#include "protocol.h"
#include "writer_thread.h"

/* Process a PUT command. */
yerr_t command_put(tcp_thread_t *thread, ybool_t sync, ybool_t compress, ydynabin_t *buff) {
	uint16_t *pname_len, name_len;
	uint32_t *pdata_len, data_len;
	void *ptr, *name = NULL, *data = NULL;
	writer_msg_t *msg = NULL;
	char answer;

	YLOG_ADD(YLOG_DEBUG, "PUT command");
	// read name length
	if (connection_read_data(thread->fd, buff, sizeof(name_len)) != YENOERR)
		goto error;
	pname_len = ydynabin_forward(buff, sizeof(name_len));
	name_len = ntohs(*pname_len);
	// read name
	if (connection_read_data(thread->fd, buff, (size_t)name_len) != YENOERR)
		goto error;
	ptr = ydynabin_forward(buff, (size_t)name_len);
	if ((name = YMALLOC((size_t)name_len)) == NULL)
		goto error;
	memcpy(name, ptr, (size_t)name_len);
	// read data length
	if (connection_read_data(thread->fd, buff, sizeof(data_len)) != YENOERR)
		goto error;
	pdata_len = ydynabin_forward(buff, sizeof(data_len));
	data_len = ntohl(*pdata_len);
	// read data
	if (data_len > 0) {
		if (connection_read_data(thread->fd, buff, (size_t)data_len) != YENOERR)
			goto error;
		ptr = ydynabin_forward(buff, (size_t)data_len);
		if ((data = YMALLOC((size_t)data_len)) == NULL)
			goto error;
		memcpy(data, ptr, (size_t)data_len);
	}

	// not synchronized: immediate response
	if (!sync)
		connection_send_response(thread->fd, RESP_OK, YFALSE, NULL, 0);

	// creation of the message
	if ((msg = YMALLOC(sizeof(writer_msg_t))) == NULL)
		goto error;
	msg->name = name;
	msg->name_len = name_len;
	msg->data = data;
	msg->data_len = data_len;
	// send the message to the writer thread
	if (nn_send(thread->write_sock, &msg, sizeof(msg), 0) < 0) {
		YLOG_ADD(YLOG_WARN, "Unable to send message to writer thread.");
		goto error;
	}
	// wait for the answer
	if (nn_recv(thread->write_sock, &answer, sizeof(answer), 0) < 0) {
		YLOG_ADD(YLOG_WARN, "Unable to get answer from writer thread.");
		goto error;
	}
	YLOG_ADD(YLOG_DEBUG, "PUT command %s", (answer ? "OK" : "failed"));
	//YFREE(name);
	//YFREE(data);
	//YFREE(msg);
	if (!sync)
		return (YENOERR);
	return (connection_send_response(thread->fd, (answer ? RESP_OK : RESP_NO_DATA),
	                                 YFALSE, NULL, 0));
error:
	YLOG_ADD(YLOG_WARN, "PUT error");
	YFREE(name);
	YFREE(data);
	YFREE(msg);
	connection_send_response(thread->fd, RESP_SERVER_ERR, YFALSE, NULL, 0);
	return (YEIO);
}
