
int template_read_reg_spi(struct spi_device *spi, int addr, int value)
{
    struct spi_message message;

    spi_sync(spi, &message);

    return 0;
}
