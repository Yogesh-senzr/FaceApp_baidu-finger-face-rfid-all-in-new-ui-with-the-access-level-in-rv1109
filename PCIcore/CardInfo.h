#include <string>
#include <sys/time.h>
#include <pthread.h>

using namespace std;
#define CARD_NUM 32

struct card_info
{
	char card_no[CARD_NUM];
	timeval time_stamp;
	bool handled;
};

class Card_Service
{
	public:      
		static Card_Service& instance();
		Card_Service();
	    ~Card_Service();
		void getCardInfo(struct card_info* data);
		void clear_CardInfo();

	private:
		static void* updateInfoThread(void *arg);
		void do_UpdateCardInfo();
	
	private:
		pthread_mutex_t m_infoMutex;
		struct card_info m_info;
		pthread_t getcard_info_tid;
};

