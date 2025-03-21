#include "CardInfo.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>


Card_Service& Card_Service::instance()
{
	static Card_Service m_instance;
	return m_instance;
}

Card_Service::Card_Service()
{
	memset(&m_info,0,sizeof(m_info));
	pthread_mutex_init(&m_infoMutex, NULL);
	// card hw init;
	pthread_create(&getcard_info_tid,NULL,Card_Service::updateInfoThread,this);
}

Card_Service::~Card_Service()
{
	pthread_join(getcard_info_tid,NULL);	

}

void* Card_Service::updateInfoThread(void *arg)
{
	Card_Service* ss = static_cast<Card_Service*>(arg);
	ss->do_UpdateCardInfo();
	return NULL;
}


void Card_Service::do_UpdateCardInfo()
{
	while(1)
	{
		//LogV("update card info.");
		pthread_mutex_lock(&m_infoMutex);
		//get card INFO
		
		gettimeofday(&m_info.time_stamp, NULL);
		m_info.handled = false;
		
		pthread_mutex_unlock(&m_infoMutex);
		sleep(1);		
	}
}

void Card_Service::getCardInfo(struct card_info* data)
{
	pthread_mutex_lock(&m_infoMutex);
	memcpy(data,&m_info,sizeof(m_info));
	pthread_mutex_unlock(&m_infoMutex);
}


void Card_Service::clear_CardInfo()
{
	pthread_mutex_lock(&m_infoMutex);
	memset(&m_info,0,sizeof(m_info));
	m_info.handled = true;
	pthread_mutex_unlock(&m_infoMutex);
}

