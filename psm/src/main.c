#include <zephyr.h>
#include <net/socket.h>
#include <stdio.h>
#include <string.h>
#include <modem/at_cmd.h>
#include <bsd.h>

#define M1 0
#define NB1 1

#define CAT M1 //M1 or NB1

#if CAT == M1
#define AT_EDRX_REQ         "AT+CEDRXS=2,4,\"0010\""
#define AT_PTW_REQ          "AT\%XPTW=4,\"0000\""
#define AT_XSYSTEMMODE		"AT\%XSYSTEMMODE=1,0,0,0"
#elif CAT == NB1
#define AT_EDRX_REQ         "AT+CEDRXS=2,5,\"0010\""
#define AT_PTW_REQ          "AT\%XPTW=5,\"0000\""
#define AT_XSYSTEMMODE		"AT\%XSYSTEMMODE=0,1,0,0"
#endif

#define AT_SUBSCRIBE		"AT+CEREG=5"
#define AT_CEREG	    	"AT+CEREG?"
#define AT_PSM_REQ			"AT+CPSMS=1,\"\",\"\",\"10101010\",\"00000000\""
#define AT_XMONITOR	    	"AT\%XMONITOR"
#define AT_CFUN1        	"AT+CFUN=1"
#define AT_POWER            "AT\%XDATAPRFL=0"

bool connected = false;
enum at_cmd_state at_state;

static void at_cmd_notification_handler(const char* response){
    printk("AT notify: \t%s",response);
    if (!memcmp("+CEREG: 1", response, 9) || !memcmp("+CEREG: 5", response, 9)){
        connected = true;
    }
}
static void at_cmd_handler(const char* response){
    if(strlen(response) > 0){
        printk("AT recv:\t%s",response);
    }
}
static void at_send_command(const char *cmd)
{
    printk("AT send: \t%s\n", cmd);
    at_cmd_write_with_callback(cmd, &at_cmd_handler, &at_state);
    if(at_state){
        printk("AT ret: \t%u\n",at_state);
    }
    else{
        printk("AT ret: \tOK\n");
    }
}
int main(void){
    at_cmd_set_notification_handler(&at_cmd_notification_handler);

	// start modem
	at_send_command(AT_POWER);
	at_send_command(AT_SUBSCRIBE);
	at_send_command(AT_XSYSTEMMODE);
	at_send_command(AT_EDRX_REQ);
	at_send_command(AT_PTW_REQ);

	at_send_command(AT_CFUN1);
    printk("connecting...\n");
    while(connected == false){k_sleep(K_MSEC(1000));}
    printk("connected\n");
    at_send_command(AT_CEREG);

    // wait 30 sec and enter PSM
    k_sleep(K_MSEC(30000));
    at_send_command(AT_PSM_REQ);
    k_sleep(K_MSEC(10000));
    at_send_command(AT_XMONITOR);

    while(true){
        k_sleep(K_MSEC(10000));
    }
}

