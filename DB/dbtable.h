#ifndef DBTABLE_H
#define DBTABLE_H

#define _INSERT_PERSON_COLUMN_GIDS(sql) \
    do{ \
    sql = "ALTER TABLE person ADD gids VARCHAR(1024) NULL  default 1;"; \
    }while(0); \

#define _INSERT_PERSON_COLUMN_CATCH_IMG(sql) \
    do{ \
    sql = "ALTER TABLE person ADD catch_img BLOB NULL;"; \
    }while(0); \

#define _INSERT_PERSON_COLUMN_TIMEOFACCESS(sql) \
    do{ \
    sql = "ALTER TABLE person ADD time_of_access VARCHAR(1024) NULL  default \'\';"; \
    }while(0); \

#define _INSERT_PERSON_COLUMN_AIDS(sql) \
    do{ \
    sql = "ALTER TABLE person ADD aids VARCHAR(1024) NULL  default 1;"; \
    }while(0); \

#define _INSERT_PERSON_COLUMN_DEPARTMENT(sql) \
    do{ \
    sql = "ALTER TABLE person ADD department VARCHAR(128) NULL  default \'\';"; \
    }while(0); \

#define _CREATE_PERSON_TABLE(sql) \
    do{ \
    sql = \
    "CREATE TABLE person (" \
    "personid          INTEGER        PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL," \
    "uuid              VARCHAR (128) NOT NULL," \
    "persontype        INT          NOT NULL," \
    "name              VARCHAR (64) NOT NULL," \
    "sex               VARCHAR (12)  NOT NULL," \
    "image             VARCHAR (256)," \
    "createtime        VARCHAR (256)  NOT NULL," \
    "idcardnum         VARCHAR (128)," \
    "iccardnum         VARCHAR (128)," \
    "feature           BLOB  ," \
    "featuresize       INT " \
    ");"; \
    }while(0); \

#define _CREATE_PASSAGETIME_TABLE(sql) \
    do{ \
    sql = \
    "CREATE TABLE PassageTime(" \
    "stateTimer VARCHAR2(64)," \
    "endTimer VARCHAR2(64), " \
    "Monday INT DEFAULT 1, " \
    "Tuesday INT DEFAULT 1," \
    "Wednesday INT DEFAULT 1, " \
    "Thursday INT DEFAULT 1," \
    "Friday INT DEFAULT 1, " \
    "Saturday INT DEFAULT 1," \
    "Sunday INT DEFAULT 1," \
    "PRIMARY KEY(stateTimer, endTimer));";\
    }while(0); \

#define DEFAULT_ACTION ("1|2")
#define ACTION_AND ("&")
#define ACTION_OR ("|")

#define _CREATE_ACTION_TABLE(sql) \
    do{ \
    sql = \
    "CREATE TABLE actions (" \
    "aid        INTEGER        PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL," \
    "code        VARCHAR (64) NOT NULL," \
    "name        VARCHAR (64) NOT NULL," \
    "desc       VARCHAR (256) NOT NULL" \
    ");"; \
    }while(0); \

#define DEFAULT_GROUP "1"
#define GROUP_AND ","

#define _CREATE_GROUP_TABLE(sql) \
    do{ \
    sql = \
    "CREATE TABLE groups (" \
    "gid        INTEGER        PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL," \
    "name        VARCHAR (64) NOT NULL," \
    "desc       VARCHAR (256) NOT NULL," \
    "aids         VARCHAR (1024)  NOT NULL" \
    ");"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_GIDS(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD gids VARCHAR(1024) NULL  default 1;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_PIDS(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD pids VARCHAR(1024) NULL  default 1;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_IDCARDNUM(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD idcardnum VARCHAR(128) NULL;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_ICCARDNUM(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD iccardnum VARCHAR(128) NULL;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_UPLOADFLAG(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD uploadflag VARCHAR(8) NULL default 0;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_TEMPVALUE(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD tempvalue VARCHAR(10) NULL default 0;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_SEX(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD sex VARCHAR(12) NULL;"; \
    }while(0); \

#define _INSERT_IDENTIFYRECORD_COLUMN_MSG(sql) \
    do{ \
    sql = "ALTER TABLE identifyrecord ADD msg VARCHAR(256) NULL;"; \
    }while(0); \

#define _CREATE_IDENTIFYRECORD_TABLE(sql) \
    do{ \
    sql = \
    "CREATE TABLE identifyrecord (" \
    "rid        INTEGER        PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL," \
    "Identifyed        INT        NOT NULL," \
    "face_mask         INT        NOT NULL," \
    "personid        INTEGER        NOT NULL," \
    "uuid        VARCHAR (128)        NOT NULL," \
    "persontype         INT        NOT NULL," \
    "name        VARCHAR (64) NOT NULL," \
    "img_path         VARCHAR (256) NOT NULL," \
    "time         VARCHAR (256)  NOT NULL" \
    ");"; \
    }while(0); \

#endif // DBTABLE_H
