/*
 * macro.h
 *
 *  Created on: Nov 21, 2016
 *      Author: root
 */

#ifndef OpenKAI_src_Base_macro_H_
#define OpenKAI_src_Base_macro_H_

#define CLI_MSG(x,y) m_cliMsgLevel=y;m_cliMsg=x;
#define LOG_I(x) if(m_bLog){LOG(INFO)<<*this->getName()<<": "<<x;}else{CLI_MSG(x,0);}
#define LOG_E(x) if(m_bLog){LOG(ERROR)<<*this->getName()<<": "<<x;}else{CLI_MSG(x,1);}
#define LOG_F(x) if(m_bLog){LOG(FATAL)<<*this->getName()<<": "<<x;}else{CLI_MSG(x,2);}

#define F_FATAL_F(x) if(x==false){LOG_F(#x);return false;}
#define F_ERROR_F(x) if(x==false){LOG_E(#x);return false;}
#define F_INFO(x) if(x==false){LOG_I(#x);}

#define KISSm(x,y) x->v(#y, &m_##y)
#define KISSm_E(x,y) F_ERROR_F(x->v(#y, &m_##y))
#define KISSm_F(x,y) F_FATAL_F(x->v(#y, &m_##y))

#define F_(x) if(x==false){return;}
#define IF_T(x) if(x){return true;}
#define IF_F(x) if(x){return false;}
#define IF_N(x) if(x){return NULL;}
#define IF_(x) if(x){return;}
#define IF__(x,y) if(x){return y;}
#define IF_CONT(x) if(x){continue;}

#define F_l(x,y) if(x==false){LOG_E(y);return;}
#define IF_Fl(x,y) if(x){LOG_E(y);return false;}
#define IF_Nl(x,y) if(x){LOG_E(y);return NULL;}
#define IF_l(x,y) if(x){LOG_E(y);return;}
#define IF_NEGl(x,y) if(x){LOG_E(y);return -1;}

#define NULL_(x) if(x==NULL){return;}
#define NULL__(x,y) if(x==NULL){return y;}
#define NULL_F(x) if(x==NULL){return false;}
#define NULL_N(x) if(x==NULL){return NULL;}
#define NULL_T(x) if(x==NULL){return true;}

#define NULL_l(x,y) if(x==NULL){LOG_E(y);return;}
#define NULL_Fl(x,y) if(x==NULL){LOG_E(y);return false;}
#define NULL_Nl(x,y) if(x==NULL){LOG_E(y);return NULL;}

#define DEL(x) if(x){delete x;x=NULL;}
#define DEL_ARRAY(x) if(x){delete[] x;x=NULL;}
#define SWAP(x,y,t) {t=x;x=y;y=t;}

#endif
