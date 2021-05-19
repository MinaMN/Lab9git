#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <linux/kfifo.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/cdev.h>


void rc4(unsigned char * p, unsigned char * k, unsigned char * c,int l)
{
        unsigned char s [256];
        unsigned char t [256];
        unsigned char temp;
        unsigned char kk;
        int i,j,x;
        for ( i  = 0 ; i  < 256 ; i ++ )
        {
                s[i] = i;
                t[i]= k[i % 4];
        }
        j = 0 ;
        for ( i  = 0 ; i  < 256 ; i ++ )
        {
                j = (j+s[i]+t[i])%256;
                temp = s[i];
                s[i] = s[j];
                s[j] = temp;
        }

        i = j = -1;
        for ( x = 0 ; x < l ; x++ )
        {
                i = (i+1) % 256;
                j = (j+s[i]) % 256;
                temp = s[i];
                s[i] = s[j];
                s[j] = temp;
                kk = (s[i]+s[j]) % 256;
                c[x] = p[x] ^ s[kk];
        }
}

char cipher[4096]="Mina";
char cipherKey [128] = "root123";
char cipherProcKey [128]= "root123";
char NcrptdBuff [4096];

struct cdev charBuff[2];

/*--------------------------------------------------------------------*/
static int cipherProcPrnt(struct seq_file *sf, void *v) {
  seq_printf(sf, "%s\n", cipher);
  return 0;
}

static int cipherProcOpen(struct inode *i, struct  file *f) {
  return single_open(f, cipherProcPrnt, NULL);
}

/*-------------------------------------------------------------------------*/
static int keyProcPrnt(struct seq_file *sf, void *v) {
  seq_printf(sf, "Go away silly one, you cannot see my key >-:y\n");
  return 0;
}

static ssize_t keyProcWrite(struct file *f , const char* cCh, size_t st, loff_t* lt){
copy_from_user(cipherProcKey,cCh,st);
  rc4(NcrptdBuff,cipherProcKey, cipher,4096);
return st; 
}

static int keyProcOpen(struct inode *i, struct  file *f) {
  return single_open(f, keyProcPrnt, NULL);
}

/*------------------------------------------------------------------------------*/
static int cipherDevPrnt(struct seq_file *sf, void *v) {
  
  seq_printf(sf, "%s\n", NcrptdBuff);
  return 0;
}

static ssize_t cipherDevWrite(struct file * f, const char* cCh, size_t st, loff_t* lt){
copy_from_user(cipher,cCh,st);

rc4(cipher,cipherKey, NcrptdBuff,4096);
return st; 
  

}

static int cipherDevOpen(struct inode *i, struct  file *f) {
  return single_open(f, cipherDevPrnt, NULL);
}
/*-------------------------------------------------------------------------------------*/
static int keyDevPrnt(struct seq_file *sf, void *v) {
  seq_printf(sf, "Go away silly one, you cannot see my key >-:\n");
  return 0;
}

static ssize_t keyDevWrite(struct file * f, const char* cCh, size_t st, loff_t* lt){
copy_from_user(cipherKey,cCh,st);
return st; 
}

static int keyDevOpen(struct inode *i, struct  file *f) {
  return single_open(f, keyDevPrnt, NULL);
}

/*---------------------------------------------------------------------------------------*/
static const struct file_operations cipherProcfops = {
  .owner = THIS_MODULE,
  .open = cipherProcOpen,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
/*---------------------------------------------------------------------------------------*/
static const struct file_operations keyProcfops = {
  .owner = THIS_MODULE,
  .open = keyProcOpen,
  .read = seq_read,
  .write=keyProcWrite,
  .llseek = seq_lseek,
  .release = single_release,
};

/*---------------------------------------------------------------------------------------*/
static const struct file_operations cipherDevfops = {
  .owner = THIS_MODULE,
  .open = cipherDevOpen,
  .read = seq_read,
  .write=cipherDevWrite,
  .llseek = seq_lseek,
  .release = single_release,
};

/*---------------------------------------------------------------------------------------*/
static const struct file_operations keyDevfops = {
  .owner = THIS_MODULE,
  .open = keyDevOpen,
  .read = seq_read,
  .write=keyDevWrite,
  .llseek = seq_lseek,
  .release = single_release,
};

/*---------------------------------------------------------------------------------------*/


int init(void){
		
	proc_create("cipher", 0644, NULL, &cipherProcfops);
	proc_create("key", 0644, NULL, &keyProcfops);
	

	cdev_init(&charBuff[0], &cipherDevfops);
	cdev_add (&charBuff[0], MKDEV (240,0),1);

	cdev_init(&charBuff[1], &keyDevfops);
	cdev_add (&charBuff[1], MKDEV(240,1),1);
	return 0; 
}



void mexit(void){

remove_proc_entry("cipher", NULL);
remove_proc_entry("key", NULL);
	cdev_del(&charBuff[0]);

	printk(KERN_ALERT "The device now is Unregistered from the cipher\n");

	cdev_del(&charBuff[1]);
	
	printk(KERN_ALERT " The device now is Unregistered from the key\n");
}

MODULE_LICENSE("GPL");
module_init(init);
module_exit(mexit);

