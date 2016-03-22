/**
 * @file      hello_file.c
 * @author    Jake Drahos
 * @date      2015-03-08: Last updated
 * @brief     Character driver example
 * @copyright MIT License (c) 2015
 */
 
/*
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

///--- Prototypes ---///
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

///--- Macros ---///
#define SUCCESS 0
#define DEVICE_NAME "cpre308"
/// The max length of the message from the device
#define BUF_LEN 80
 
///--- Global Variables ---///
static int Major;			// Major number assigned to this device driver
static int Device_Open = 0;	// is the device open already?
static char buf[BUF_LEN];	// The message the device will return
static char *buf_ptr;		// Position for readback

///--- Register Callbacks ---///
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

///--- Init Module ---///
int __init device_init(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	
	if(Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");

	buf_ptr = buf;

	return SUCCESS;
}

///--- Cleanup Module ---///
void __exit device_exit(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
} 

///--- register init and exit functions ---///
module_init(device_init);
module_exit(device_exit);

///--- Device Open ---///
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;
	
	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

///--- Device Release ---///
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}

///--- Device Read ---///
static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	ssize_t bytes_read = 0;
	
	/* Read back-to-front until buf_ptr is at the beginning of the stored message.
	 * This will cause it to return 0 if the buf_ptr points to the
	 * beginning of the message already. This covers two cases: Nothing has
	 * been written yet, and everything has already been read. Note that
	 * this means reads are destructive: Once read and reversed, data must
	 * be written before it can be read again
	 */
	while (length && (buf_ptr != buf)) {
		put_user(*(--buf_ptr), buffer++);
		
		length--;
		bytes_read++;
	}
	
	return bytes_read;
}

///--- Device Write ---///
static ssize_t device_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t bytes_written = 0;

	/* Copy len (or BUF_LEN) bytes into buf */
	while (len && (bytes_written <= BUF_LEN)) {
		get_user(*(buf_ptr++), buffer++);
		len--;
		bytes_written++;
	}

	return bytes_written;
}
