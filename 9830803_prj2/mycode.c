#include <linux/init.h> // For module init and exit
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // For fops
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/slab.h>
//#include <string.h> // Can't use it!

#define DEVICE_NAME "bank"
#define MAX 100

MODULE_LICENSE("GPL");

// FILE OPERATIONS
static int bank_open(struct inode*, struct file*);
static int bank_release(struct inode*, struct file*);
static ssize_t bank_read(struct file*, char*, size_t, loff_t*);
static ssize_t bank_write(struct file*, const char*, size_t, loff_t*);
static struct file_operations fops = {
   .open = bank_open,
   .read = bank_read,
   .release = bank_release,
   .write = bank_write
};

// Why "static"? --> To bound it to the current file.
static int major; // Device major number. Driver reacts to this major number.
static int account[MAX];
struct mutex lock;
// Event --> LOAD
static int __init bank_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops); // 0: dynamically assign a major number ||| name is displayed in /proc/devices ||| fops.
    if (major < 0) {
        printk(KERN_ALERT "bank module load failed.\n");
        return major;
    }
    else{
    	int i=0;
    	for ( i=0;i<MAX;i++)account[i]=2000000;
    	mutex_init(&lock);
    	printk(KERN_INFO "bank module loaded: %d\n", major);
    
    }
    printk(KERN_INFO "bank module loaded: %d\n", major);
    return 0;
}

// Event --> UNLOAD
static void __exit bank_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "bank device module unloaded.\n");
}

// Event --> OPEN
static int bank_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "bank device opened.\n");
   return 0;
}

// Event --> CLOSE
static int bank_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "bank device closed.\n");
   return 0;
}

// Event --> READ
static ssize_t bank_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    char* message;
    message = kmalloc(2000, GFP_KERNEL);
    char temp[256];
    int i=0;
    for(i=0;i<MAX;i++){
    	//char acc[256];
    	//acc[0]='[';
    	sprintf(temp,"%d,",account[i]);
 //   	printk(KERN_INFO "account: %s\n",message);
    	strcat(message,temp);  
    }
    
    int errors = 0;
    mutex_lock(&lock);
    errors = copy_to_user(buffer, message, strlen(message));
    mutex_unlock(&lock);
    return errors == 0 ? strlen(message) : -EFAULT;
}


static ssize_t bank_write(struct file* filep, const char *buffer , size_t len, loff_t* offset){
	char input[20];
	long from,to,amount;
	char* temp = kmalloc(10,GFP_KERNEL);
	char* temp2 = kmalloc(10,GFP_KERNEL);
	char* temp3 = kmalloc(10,GFP_KERNEL);
	mutex_lock(&lock);
	sprintf(input,"%s",buffer);
	
	
	if(input[0]=='r'){
	int i=0;
		for(i=0;i<MAX;i++)account[i]=2000000;
		printk(KERN_INFO "all balances have been reset\n");
	}
	else{
	if(input[4]==','){
		strncat(temp,&input[2],1);
		strncat(temp,&input[3],1);
		kstrtol(temp,10,&from);
	}
	else{
		if(input[2]!='-'){
		strncat(temp,&input[2],1);
		kstrtol(temp,10,&from);
		}
		else{
		from=-1;
		}
	
	}
	
	if(input[7]==','){
		strncat(temp2,&input[5],1);
		strncat(temp2,&input[6],1);
		kstrtol(temp2,10,&to);
	
	}
	else if(input[6]==','){
		if(input[3]==','){
		strncat(temp2,&input[4],1);
		strncat(temp2,&input[5],1);
		}
		else strncat(temp2,&input[5],1);
		kstrtol(temp2,10,&to);
	}
	else{
		if(input[5]!='-'&&input[4]!='-'){
		strncat(temp2,&input[4],1);
		kstrtol(temp2,10,&to);
		}
		else{
		to=-1;
		}
	}
///	printk(KERN_INFO "from:%d to:%d\n",from,to);
	
	long lastocc;
	char* last= kmalloc(10,GFP_KERNEL);
	last = strrchr(input,',');
	//kstrtol(last,10,&lastocc);
//		printk(KERN_INFO "last occ :%s\n",last);
	if(last[0]==',')last++;
	kstrtol(last,10,&amount);

	if(input[0]=='e'){
		if(amount>account[from]){
			printk(KERN_INFO "insufficient funds\n");
			return 0;
		}
		account[from] = account[from]-amount;
		account[to] = account[to]+amount;
		printk(KERN_INFO "%d units transfered from account %d to account %d\n",amount,from,to);
	}
	else if(input[0]=='v'){
		account[to] = account[to]+amount;
		printk(KERN_INFO "%d units deposited into account %d\n",amount,to);
	}
	else if(input[0]=='b'){
		if(amount>account[from]){
			printk(KERN_INFO "insufficient funds\n");
			return 0;
		}
		account[from] = account[from]-amount;
		printk(KERN_INFO "%d units withdrawed from account %d\n",amount,from);
	}}
	mutex_unlock(&lock);
	return 0;
}

// Registering load and unload functions.
module_init(bank_init);
module_exit(bank_exit);

