#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)
#define msleep(x) usleep((x)*1000)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    if (thread_param)
    {
        struct thread_data* p_data = (struct thread_data*)thread_param;
	p_data->thread_complete_success = false;
        
	int ret = msleep(p_data->wait_to_obtain_ms);
	if (ret != 0) 
	{
	    ERROR_LOG("failed to wait for %d before obtaining mutex\n", p_data->wait_to_obtain_ms);
	    goto exit;	
	}

	ret = pthread_mutex_lock(p_data->mutex);
	if (ret != 0)
	{
	    ERROR_LOG("failed to lock mutex\n");
	    goto exit;	
	}
	
	ret = msleep(p_data->wait_to_release_ms);
	if (ret != 0) 
	{
	    ERROR_LOG("failed to wait for %d before releasing mutex\n", p_data->wait_to_release_ms);
	    goto exit;	
	}
	
	ret = pthread_mutex_unlock(p_data->mutex);
	if (ret != 0)
	{
	    ERROR_LOG("failed to unlock mutex\n");
	    goto exit;	
	}
        
	p_data->thread_complete_success = true;
    }

exit: 
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    if (!thread || !mutex) 
    {
        ERROR_LOG("Invalid paramters\n");
	return false;
    }
    
    struct thread_data* p_data = (struct thread_data*) malloc(sizeof(struct thread_data)); 

    if (!p_data) 
    {
        ERROR_LOG("Failed to allocate memory\n");
    } 
    else 
    {
        p_data->wait_to_obtain_ms = wait_to_obtain_ms;
	p_data->wait_to_release_ms = wait_to_release_ms;
        p_data->mutex = mutex;

	int ret = pthread_create(thread, NULL, threadfunc, (void*)p_data);
	if (ret) 
	{ // 0 on success
            ERROR_LOG("Failed to create a thread\n");
	} 
	else 
	{
            DEBUG_LOG("A thread is created successfully\n");
	    return true;
	}
    }

    return false;
}

