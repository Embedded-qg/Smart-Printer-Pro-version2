/**
 * @name        void receive_connection(struct netconn *conn)
 * @brief       从网络缓冲区读取数据并解析其是否是一个正确的批次，是则根据是否为加急存入相应的本地缓冲区，否则丢弃
 * @parameter   struct netconn *conn:全局的TCP链接
 * @description 调用该函数的线程优先级应高于进入打印队列线程的优先级，目的是为了保证缓冲区尽可能满
 */
void receive_connection(struct netconn *conn)
{
    struct netbuf *order_netbuf = NULL;
    extern OS_EVENT *Print_Queue_Sem;
    extern OS_EVENT *Recon_To_Server_Sem;
    err_t err;
    u8_t hash;//批次哈希值

    while((err = netconn_recv(conn, &order_netbuf)) == ERR_OK)
    {       
        char *data;
start:
        while(batch_head_current_len < MAX_BATCH_HEAD_LENGTH)
        { 
            netbuf_data(order_netbuf, (void **)&data, &len);//从网络缓冲区读取
            /*考虑上一个缓冲区读取完一个批次后还剩下一些数据，故需要重新读取这个缓冲区，并加上被读取过后的偏移量，目的是让data指针指向未读的数据*/
            data += leave_len;
            len -= leave_len;

            while(len > 0 && batch_head_current_len < MAX_BATCH_HEAD_LENGTH)
            {
                if(batch[0] != '\xaa'){//一直找寻不到批次头，所以一直寻找
                    find_substr_head(&data, "\xaa\x55", &len, 2);
                }

                if(len > 0){//len大于0代表检测到批次头并且data指针还指向的缓冲区还有剩余容量
                    batch[batch_head_current_len++] = *data++;
                    count++;                    
                    if(batch_head_current_len == MAX_BATCH_HEAD_LENGTH){
                        if(*(batch + BATCH_TAIL_OFFSET) == '\x55' && *(batch + BATCH_TAIL_OFFSET + 1) == '\xaa'){//表示是批次尾
                            int i;
                            flag = 1;
                            ANALYZE_DATA_2B((batch + BATCH_NUMBER_OFFSET), batch_number);//获取批次号
                            Analyze_Batch_Info_Table(batch, batch_number);//批次解包

                            hash = get_batch_hash(batch_number);
                            len -= count;

                            batch_total_len = 20 + len;
                            if (batch_total_len >= batch_info_table[hash].batch_length){
                                u16_t sub_len;
                                sub_len =  len - (batch_total_len - batch_info_table[hash].batch_length);//从网络缓冲区读取到多于一个批次的数据，则只保留前面的数据

                                if(batch_number != last_batch_number){
                                    put_in_buf(data, sub_len, batch_info_table[hash].preservation);//保存订单数据至缓冲区

                                    /*释放信号量，通知打印队列有新的批次数据*/
                                    OSSemPost(Print_Queue_Sem);
                                    OSSemPost(Batch_Rec_Sem);
                                }

                                begin_new_batch();
                                leave_len = len - sub_len;                  

                                NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//发送批次状态，进入缓冲区

                                if(leave_len > 0)//若有剩余数据则继续返回检测是否是一个新的批次
                                    goto start;
                                else
                                    break;

                            }else{
                                if(batch_number != last_batch_number)
                                    put_in_buf(data, len, batch_info_table[hash].preservation);//保存订单数据至缓冲区
                            }
                        }else{
                            begin_new_batch();
                        }
                    }
                    if(count == len){//数据读取完，结束操作
                        count = 0;
                        len = 0;
                        break;
                    }
                }
            }
            if(!(netbuf_next(order_netbuf) > 0)){
                goto delete;
            }
        }

        do
        {
            if(flag == 0)//1表示已经读取了批次头，否则没有
                goto start;

            netbuf_data(order_netbuf, (void **)&data, &len);   
            batch_total_len += len;

            if (batch_total_len >= batch_info_table[hash].batch_length){
                u16_t sub_len;
                sub_len =  len - (batch_total_len - batch_info_table[hash].batch_length);//从网络缓冲区读取到多于一个批次的数据，则只保留前面的数据

                if(batch_number != last_batch_number){
                    put_in_buf(data, sub_len, batch_info_table[hash].preservation);//保存订单数据至缓冲区

                    /*释放信号量，通知打印队列有新的批次数据*/
                    OSSemPost(Print_Queue_Sem);
                    OSSemPost(Batch_Rec_Sem);
                }
                begin_new_batch();
                NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//发送批次状态，进入缓冲区
                leave_len = len - sub_len;                                                          
                if(leave_len > 0)//若有剩余数据则继续返回检测是否是一个新的批次
                    goto start;
            }
            else{
                if(batch_number != last_batch_number)
                    put_in_buf(data, len, batch_info_table[hash].preservation);//保存普通订单数据至缓冲区
            }
        }while(netbuf_next(order_netbuf) > 0);
delete:
        netbuf_delete(order_netbuf);
    }
}

