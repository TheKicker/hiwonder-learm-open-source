#include "Hiwonder.hpp"
#include <Arduino.h>
#include <SPI.h>
#include "Config.h"

#define NORFLASH_CLK_PIN        FLASH_CLK    
#define NORFLASH_MOSI_PIN       FLASH_DI   
#define NORFLASH_MISO_PIN       FLASH_DO
#define NORFLASH_CS_PIN         FLASH_CS

// #define NORFLASH_DEBUG_ENABLE  // uart debug
#define HARDWARE_SPI           // use hardware spi
// #define SOFTWARE_SPI           // use software spi

#define NORFLASH_HOLD_PIN       -1     // hold pin connect 3.3V
#define NORFLASH_WP_PIN         -1     // wp pin connect 3.3V

#define ManufactDeviceID_CMD	0x90   
#define READ_JEDEC_ID_CMD       0x9F
#define WRITE_STATUS            0x01    
#define READ_STATU_REGISTER_1   0x05    
#define READ_STATU_REGISTER_2   0x35
#define READ_DATA_CMD	        0x03
#define WRITE_ENABLE_CMD	    0x06
#define WRITE_DISABLE_CMD	    0x04
#define SECTOR_ERASE_CMD	    0x20
#define CHIP_ERASE_CMD	        0xC7
#define PAGE_PROGRAM_CMD        0x02

#define ONE_PAGE_SIZE           256     
#define SPI_FREQUENCY           1000000

/* Norflash write one byte */
void write_byte(uint8_t data)
{
#ifdef HARDWARE_SPI
    // SPI.transfer(data);
    SPI.write(data);
#else if SOFTWARE_SPI
    for(uint8_t i = 0; i < 8; i++)
    {     
        uint8_t status;
        status = data & (0x80 >> i);
        digitalWrite(NORFLASH_MOSI_PIN, status);
        digitalWrite(NORFLASH_CLK_PIN, LOW);
        digitalWrite(NORFLASH_CLK_PIN, HIGH);   
    }
#endif
}

/* Norflash write enable */
void write_enable()
{
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(WRITE_ENABLE_CMD);
    digitalWrite(NORFLASH_CS_PIN, HIGH);
}

/* Norflash read one byte */
uint8_t read_byte(uint8_t tx_data)
{
#ifdef HARDWARE_SPI
    uint8_t data = 0;
    data = SPI.transfer(tx_data);
    return data;
#else if SOFTWARE_SPI
    uint8_t i = 0, data = 0;
    for(i = 0; i < 8; i++)
    {
        digitalWrite(NORFLASH_CLK_PIN, HIGH);
        digitalWrite(NORFLASH_CLK_PIN, LOW);
        if(digitalRead(NORFLASH_MISO_PIN))
        {
            data |= 0x80 >> i;
        }
    }
    return data;
#endif
}

/* Read norflash status */
uint8_t read_status()
{
    uint8_t status = 0; 
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(READ_STATU_REGISTER_1);      
    status = read_byte(0);                      
    digitalWrite(NORFLASH_CS_PIN, HIGH);  
    return status;
}

/* Read norflash id */
uint16_t read_norflash_id()
{
    uint8_t data = 0;
    uint16_t device_id = 0;

    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(ManufactDeviceID_CMD);
    write_byte(0x00);
    write_byte(0x00);
    write_byte(0x00);
    data = read_byte(0);
    device_id |= data;  // low byte
    data = read_byte(0);
    device_id |= (data << 8);  // high byte
    digitalWrite(NORFLASH_CS_PIN, HIGH);
    
    return device_id;
}

/* Norflash spi init */
void norflash_spi_init()
{
    // gpio init
    pinMode(NORFLASH_CS_PIN, OUTPUT);  
    digitalWrite(NORFLASH_CS_PIN, HIGH);

#ifdef HARDWARE_SPI
    SPI.begin(NORFLASH_CLK_PIN, NORFLASH_MISO_PIN, NORFLASH_MOSI_PIN, NORFLASH_CS_PIN);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setFrequency(SPI_FREQUENCY);
#else
    pinMode(NORFLASH_CLK_PIN, OUTPUT);  
    pinMode(NORFLASH_MOSI_PIN, OUTPUT); 
    pinMode(NORFLASH_MISO_PIN, INPUT); 
    digitalWrite(NORFLASH_CLK_PIN, LOW);
    delay(1);
#endif

    // check write enable status
    uint8_t data = 0;
    write_enable();                
    data = read_status();
#ifdef NORFLASH_DEBUG_ENABLE
    Serial.printf("norflash write enable status:");
    Serial.println(data, BIN);
#endif

    // read device id
    uint16_t device_id = 0;
    device_id = read_norflash_id();
#ifdef NORFLASH_DEBUG_ENABLE
    Serial.printf("norflash device id: 0x%04X", device_id);
#endif
}







/* Norflash write disable */
void write_disable()
{
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(WRITE_DISABLE_CMD);
    digitalWrite(NORFLASH_CS_PIN, HIGH);
}



/* check norflash busy status */
void check_busy(char *str)
{
    while(read_status() & 0x01)
    {
    #ifdef NORFLASH_DEBUG_ENABLE
        Serial.printf("status = 0, flash is busy of %s\n", str);   
    #endif 
    }
}

/* Write less than oneblock(256 byte) data */
void write_one_block_data(uint32_t addr, uint8_t * pbuf, uint16_t len)
{
    uint16_t i; 
    check_busy("write_one_block_data");
    write_enable();
    digitalWrite(NORFLASH_CS_PIN, LOW);           
    write_byte(PAGE_PROGRAM_CMD);                 
    write_byte((uint8_t)(addr >> 16));             
    write_byte((uint8_t)(addr >> 8));   
    write_byte((uint8_t)addr);      
    for(i = 0; i < len; i++)
    {
        write_byte(*pbuf++);   
    }
    digitalWrite(NORFLASH_CS_PIN, HIGH);                              
    check_busy("write_one_block_data");
} 

/* Write less than one sector(4096 byte) length data  */
void write_one_sector_data(uint32_t addr, uint8_t * pbuf, uint16_t len)
{ 
    uint16_t free_space, head, page, remain;  
    free_space = ONE_PAGE_SIZE - addr % ONE_PAGE_SIZE;  
    if(len <= free_space) 
    {
        head = len;
        page = 0;                      
        remain = 0; 
    }
    if(len > free_space) 
    {
        head = free_space;
        page = (len - free_space) / ONE_PAGE_SIZE;     
        remain = (len - free_space) % ONE_PAGE_SIZE;
    }
    
    if(head != 0)
    {
    #ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("head:");
        Serial.println(head);
    #endif
        write_one_block_data(addr, pbuf, head);
        pbuf = pbuf + head;
        addr = addr + head;
    }
    if(page != 0) 
    {
    #ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("page:");
        Serial.println(page);
    #endif
        for(uint16_t i = 0; i < page; i++) 
        {
            write_one_block_data(addr, pbuf, ONE_PAGE_SIZE);
            pbuf = pbuf + ONE_PAGE_SIZE;
            addr = addr + ONE_PAGE_SIZE;
        }
    }
    if(remain != 0) 
    {
    #ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("remain:");
        Serial.println(remain);
    #endif
        write_one_block_data(addr, pbuf, remain);
    }  
}

/* Read arbitrary length data */
void read_data(uint32_t addr24, uint8_t *pbuf, uint32_t len)
{
    check_busy("read_data");
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(READ_DATA_CMD);
    write_byte((uint8_t)(addr24 >> 16));   
    write_byte((uint8_t)(addr24 >> 8));   
    write_byte((uint8_t)addr24);  
    for(uint32_t i = 0; i < len; i++)
    {
        *pbuf = read_byte(0xFF);
        pbuf ++;
    }
    digitalWrite(NORFLASH_CS_PIN, HIGH); 
}


/* Erase sector */
void sector_erase(uint32_t addr24)
{
    // addr24 *= 4096;
    check_busy("sector_erase 2");
    write_enable();                     
    
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(SECTOR_ERASE_CMD);          
    write_byte((uint8_t)(addr24 >> 16));    
    write_byte((uint8_t)(addr24 >> 8));   
    write_byte((uint8_t)addr24);  
      
    digitalWrite(NORFLASH_CS_PIN, HIGH);
    check_busy("sector_erase");
} 

/* Write arbitrary length data */
void write_arbitrary_data(uint32_t addr, uint8_t* pbuf, uint32_t len)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
    uint8_t *write_buf = pbuf;	 
    uint8_t save_buffer[4096];  // save sector original data and add new data  
 	secpos = addr / 4096;       // sector number  
	secoff = addr % 4096;       // sector offset
	secremain = 4096 - secoff;  // sector remaining space   
    
 	if(len <= secremain)
    {
        secremain = len;        // sector remaining space less than 4096
    }
	while(1) 
	{	
		read_data(secpos * 4096, save_buffer, 4096); // read sector data
		for(i = 0; i < secremain; i++)
		{// check data, if all data is 0xFF no need erase sector
			if(save_buffer[secoff + i] != 0XFF)
            {// need erase sector
                break;	  
            }
		}
		if(i < secremain)
		{// erase sector and write data
			sector_erase(secpos);
			for(i = 0; i < secremain; i++)	       
			{
				save_buffer[i + secoff] = write_buf[i];	 // add new data 
			}
			write_one_sector_data(secpos * 4096, save_buffer, 4096); // write sector
		}
        else 
        {// no need erase sector
            write_one_sector_data(addr, write_buf, secremain);	
        }			   
		if(len == secremain)
        {// write done
            break; 
        }
		else
		{// continue write
			secpos ++;  // sector number + 1
			secoff = 0; // sector offset = 0 	 

		   	write_buf += secremain;  // write buff offset
			addr += secremain;       // addr offset	   
		   	len -= secremain;		 // remaining data len
			if(len > 4096)
            {// remaining data more than one sector(4096 byte)
                secremain = 4096;	 
            }
			else 
            {// remaining data less than one sector(4096 byte)
                secremain = len;			
            }
		}	 
	}	 
}

uint8_t read_act[100] = {0,0,0};

void Flash_ctl_t::init(void)
{
    norflash_spi_init(); 
}

void Flash_ctl_t::read(uint32_t addr24, uint8_t *buffer, uint32_t size24)
{
  read_data(addr24, buffer, size24);
}

void Flash_ctl_t::write(uint32_t addr24, uint8_t *buffer, uint32_t size24)
{
  write_one_block_data(addr24 , buffer , size24);
}

void Flash_ctl_t::erase_sector(uint32_t addr24)
{
  sector_erase(addr24);
}

void Flash_ctl_t::erase_all(void)
{
  uint8_t frame[256];
  memset(frame , 0, 256);
	sector_erase(MEM_FRAME_INDEX_SUM_BASE);
	write(MEM_FRAME_INDEX_SUM_BASE,frame,256);
}



