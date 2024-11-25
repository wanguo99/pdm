
int template_read_reg_i2c(struct i2c_client *client, int addr, int value)
{

    struct i2c_msg msgs;
    int num;

    i2c_transfer(client->adapter, &msgs, 1);

    return 0;
}
