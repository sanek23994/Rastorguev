#include <string.h>                               /*вкл.строковые подпрогр. */
#include <stdlib.h>                               /*вкл.подпрогр.преобр.данн*/
#include <stdio.h>                                /*вкл.подпр.станд.вв/выв  */
#include <ctype.h>                                /*вкл.подпр.классиф.симв. */

#define DL_ASSTEXT 30
#define DL_OBJTEXT 50                             /*длина об'ектн. текста   */
#define NSYM 10                                   /*размер табл.символов    */
#define NPOP 6                                    /*размер табл.псевдоопер. */
#define NOP  7                                    /*размер табл.операций    */

#define dbg(fmt, args...) printf("<<<DEBUG>>>%s[%u]"fmt"\n", __FUNCTION__, __LINE__, ##args);
#define err(fmt, args...) printf("<<<ERROR>>>%s[%u]"fmt"\n", __FUNCTION__, __LINE__, ##args);

/*
******* Б Л О К  об'явлений статических рабочих переменных
*/

char NFIL [30] = "\x0";

unsigned char PRNMET = 'N';                       /*индикатор обнаруж.метки */
int I3;                                           /*счетчик цикла           */

/*
***** Б Л О К  об'явлений прототипов обращений к подпрограммам 1-го просмотра
*/

/* прототип обращ. к подпр.обр.пс.опер.DC */
int FDC();

/* прототип обращ. к подпр.обр.пс.опер.DS */
int FDS();

/* прототип обращ. к подпр.обр.пс.опер.END */
int FEND();

/* прототип обращ.к подпр.обр.пс.опер.EQU */
int FEQU();

/* прототип обращ.к подпр.обр.пс.опер.START */
int FSTART();

/* прототип обращ.к подпр.обр.пс.опер.USING */
int FUSING();

/* прототип обращ.к подпр.обр.опер.RR-форм. */
int FRR();

/* прототип обращ.к подпр.обр.опер.RX-форм. */
int FRX();

/*
***** Б Л О К  об'явлений прототипов обращений к подпрограммам 2-го просмотра
*/

/* прототип обращ.к подпр.обр.пс.опер.DC */
int SDC();

/* прототип обращ.к подпр.обр.пс.опер.DS */
int SDS();

/* прототип обращ.к подпр.обр.пс.опер.END */
int SEND();

/* прототип обращ.к подпр.обр.пс.опер.EQU */
int SEQU();

/* прототип обращ.к подпр.обр.пс.опер.START */
int SSTART();

/* прототип обращ.к подпр.обр.пс.опер.USING */
int SUSING();

/* прототип обращ.к подпр.обр.опер.RR-форм. */
int SRR();

/* прототип обращ.к подпр.обр.опер.RX-форм. */
int SRX();

/*
******* Б Л О К  об'явлений таблиц базы данных компилятора
*/

/*
******* ОБ'ЯВЛЕНИЕ структуры строки (карты) исходного текста
*/

struct ASSKARTA                                  /*структ.карты АССЕМБЛЕРА */
{
    unsigned  char METKA    [ 8];                  /*поле метки              */
    unsigned  char PROBEL1  [ 1];                  /*пробел-разделитель      */
    unsigned  char OPERAC   [ 5];                  /*поле операции           */
    unsigned  char PROBEL2  [ 1];                  /*пробел-разделитель      */
    unsigned  char OPERAND  [12];                  /*поле операнда           */
    unsigned  char PROBEL3  [ 1];                  /*пробел разделитель      */
    unsigned  char COMM     [52];                  /*поле комментария        */
};

/*
******* НАЛОЖЕНИЕ структуры карты исх. текста на входной буфер
*/

union                                            /*определить об'единение  */
{
    unsigned char BUFCARD [80];                    /*буфер карты.исх.текста  */
    struct ASSKARTA STRUCT_BUFCARD;                /*наложение шабл.на буфер */
}   TEK_ISX_KARTA;

/*
***** СЧЕТЧИК относительного адреса (смещешия относительно базы )
*/

int CHADR;                                       /*счетчик                 */

/*
***** ТАБЛИЦА символов
*/

 int ITSYM = -1;                                  /*инд.своб.стр. табл.симв.*/

struct TSYM                                      /*структ.строки табл.симв.*/
{
    unsigned char IMSYM [8];                       /*имя символа             */
    int           ZNSYM;                           /*значение символа        */
    int           DLSYM;                           /*длина символа           */
    char          PRPER;                           /*признак перемещения     */
};

struct TSYM T_SYM [NSYM];                        /*определение табл.симв.  */

/*
***** ТАБЛИЦА машинных операций
*/

 struct TMOP                                      /*структ.стр.табл.маш.опер*/
  {
   unsigned char MNCOP [5];                       /*мнемокод операции       */
   unsigned char CODOP    ;                       /*машинный код операции   */
   unsigned char DLOP     ;                       /*длина операции в байтах */
   int (*BXPROG)()        ;                       /*указатель на подпр.обраб*/
  } T_MOP [NOP]  =                                /*об'явление табл.маш.опер*/
    {
     {{'B','A','L','R',' '} , '\x05' , 2 , FRR} , /*инициализация           */
     {{'B','C','R',' ',' '} , '\x07' , 2 , FRR} , /*строк                   */
     {{'S','T',' ',' ',' '} , '\x50' , 4 , FRX} , /*таблицы                 */
     {{'L',' ',' ',' ',' '} , '\x58' , 4 , FRX} , /*машинных                */
     {{'A',' ',' ',' ',' '} , '\x5A' , 4 , FRX} , /*операций                */
     {{'S',' ',' ',' ',' '} , '\x5B' , 4 , FRX} , /*                        */
     {{'L','H',' ',' ',' '} , '\x48' , 4 , FRX} ,
    };

/*
***** ТАБЛИЦА псевдоопераций
*/

 struct TPOP                                      /*структ.стр.табл.пс.опeр.*/
  {
   unsigned char MNCPOP[5];                       /*мнемокод псевдооперации */
   int (*BXPROG) ()       ;                       /*указатель на подпр.обраб*/
  } T_POP [NPOP] =                                /*об'явление табл.псевдооп*/
    {
     {{'D','C',' ',' ',' '} , FDC   },            /*инициализация           */
     {{'D','S',' ',' ',' '} , FDS   },            /*строк                   */
     {{'E','N','D',' ',' '} , FEND  },            /*таблицы                 */
     {{'E','Q','U',' ',' '} , FEQU  },            /*псевдоопераций          */
     {{'S','T','A','R','T'} , FSTART},            /*                        */
     {{'U','S','I','N','G'} , FUSING}             /*                        */
    };

/*
***** ТАБЛИЦА базовых регистров
*/

 struct TBASR                                     /*структ.стр.табл.баз.рег.*/
  {
   int SMESH;                                     /*                        */
   char PRDOST;                                   /*                        */
  } T_BASR[15] =                                  /*                        */
    {
     {0x00,'N'},                                  /*инициализация           */
     {0x00,'N'},                                  /*строк                   */
     {0x00,'N'},                                  /*таблицы                 */
     {0x00,'N'},                                  /*базовых                 */
     {0x00,'N'},                                  /*регистров               */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'},                                  /*                        */
     {0x00,'N'}                                   /*                        */
    };

/*
***** Б Л О К   об'явления массива с об'ектным текстом
*/

  unsigned char OBJTEXT [DL_OBJTEXT][80];         /*массив об'ектных карт   */
  int ITCARD = 0;                                 /*указатель текущ.карты   */

  struct OPRR                                     /*структ.буф.опер.форм.RR */
   {
    unsigned char OP;                             /*код операции            */
    unsigned char R1R2;                           /*R1 - первый операнд     */
						  /*R2 - второй операнд     */
   };

  union                                           /*определить об'единение  */
   {
    unsigned char BUF_OP_RR [2];                  /*оределить буфер         */
    struct OPRR OP_RR;                            /*структурировать его     */
   } RR;

  struct OPRX                                     /*структ.буф.опер.форм.RX */
   {
    unsigned char OP;                             /*код операции            */
    unsigned char R1X2;                           /*R1 - первый операнд     */
    short B2D2;                                     /*X2 - второй операнд     */
//    int B2D2;                                     /*X2 - второй операнд     */
						  /*B2 - баз.рег.2-го оп-да */
						  /*D2 - смещен.относит.базы*/
   };

  union                                           /*определить об'единение  */
   {
    unsigned char BUF_OP_RX [4];                  /*оределить буфер         */
    struct OPRX OP_RX;                            /*структурировать его     */
   } RX;

  struct STR_BUF_ESD                              /*структ.буфера карты ESD */
   {
    unsigned char POLE1      ;                    /*место для кода 0x02     */
    unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
    unsigned char POLE3  [ 6];                    /*пробелы                 */
    unsigned char POLE31 [ 2];                    /*длина данных на карте   */
    unsigned char POLE32 [ 2];                    /*пробелы                 */
    unsigned char POLE4  [ 2];                    /*внутр.ид-р имени прогр. */
    unsigned char IMPR   [ 8];                    /*имя программы           */
    unsigned char POLE6      ;                    /*код типа ESD-имени      */
    unsigned char ADPRG  [ 3];                    /*относит.адрес программы */
    unsigned char POLE8      ;                    /*пробелы                 */
    unsigned char DLPRG  [ 3];                    /*длина программы         */
    unsigned char POLE10 [40];                    /*пробелы                 */
    unsigned char POLE11 [ 8];                    /*идентификационное поле  */
   };

 struct STR_BUF_TXT                               /*структ.буфера карты TXT */
   {
    unsigned char POLE1      ;                    /*место для кода 0x02     */
    unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
    unsigned char POLE3      ;                    /*пробел                  */
    unsigned char ADOP   [ 3];                    /*относит.адрес опреации  */
    unsigned char POLE5  [ 2];                    /*пробелы                 */
    unsigned char DLNOP  [ 2];                    /*длина операции          */
    unsigned char POLE7  [ 2];                    /*пробелы                 */
    unsigned char POLE71 [ 2];                    /*внутренний идент.прогр. */
    unsigned char OPER   [56];                    /*тело операции           */
    unsigned char POLE9  [ 8];                    /*идентификационное поле  */
   };

struct STR_BUF_END                                /*структ.буфера карты END */
   {
    unsigned char POLE1      ;                    /*место для кода 0x02     */
    unsigned char POLE2  [ 3];                    /*поле типа об'ектн.карты */
    unsigned char POLE3  [68];                    /*пробелы                 */
    unsigned char POLE9  [ 8];                    /*идентификационное поле  */
   };

  union                                           /*определить об'единение  */
   {
    struct STR_BUF_ESD STR_ESD;                   /*структура буфера        */
    unsigned char BUF_ESD [80];                   /*буфер карты ESD         */
   } ESD;


  union                                           /*определить об'единение  */
   {
    struct STR_BUF_TXT STR_TXT;                   /*структура буфера        */
    unsigned char BUF_TXT [80];                   /*буфер карты TXT         */
   } TXT;

  union                                           /*определить об'единение  */
   {
    struct STR_BUF_END STR_END;                   /*структура буфера        */
    unsigned char BUF_END [80];                   /*буфер карты ESD         */
   } END;


/*
******* Б Л О К  об'явлений подпрограмм, используемых при 1-ом просмотре
*/

/* подпр.обр.пс.опер.DC */
int FDC()
{
    /* если псевдооп.DC помеч.*/
    if ( PRNMET == 'Y' )
    {
        /* если псевдооперация DC определяет константу типа F, то выполнить следующее: */
        if (TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='F')
        {
            /* уст.длину симв. =  4, */
            T_SYM[ITSYM].DLSYM = 4;
            /* а, призн.перемест.='R' */
            T_SYM[ITSYM].PRPER = 'R';

            /* если CHADR не указ. на границу слова, то: */
            if ( CHADR % 4 )
            {
                /*  уст.CHADR на гр.сл. */
                CHADR = (CHADR /4 + 1) * 4;
                /* запомн. в табл.симв. */
                T_SYM[ITSYM].ZNSYM = CHADR;
            }
            /* занулить PRNMET зн.'N'*/
            PRNMET = 'N';
        }
        else
        {
            /* иначе выход по ошибке */
            return (1);
        }
    }
    /* если же псевдооп.непомеч */
    else
    {
        /* CHADR не кратен 4,то: */
        if ( CHADR % 4 )
        {
            /* установ.CHADR на гр.сл.*/
            CHADR = (CHADR /4 + 1) * 4;
        }
    }

    /* увелич.CHADR на 4 и */
    CHADR = CHADR + 4;
    return (0);                                     /*успешно завершить подпр.*/
}

/* подпр.обр.пс.опер.DS */
int FDS()
{
    /* если псевдооп.DC помеч.,*/
    if ( PRNMET == 'Y' )
    {
        /* если псевдооперация DC определяет константу типа F, то выполнить следующее: */
        if (TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0] == 'F')
        {
            /* уст.длину симв. =  4, */
            T_SYM[ITSYM].DLSYM = 4;
            /* а,призн.перемест.='R' */
            T_SYM[ITSYM].PRPER = 'R';
            /* если CHADR не указ. на границу слова, то:*/
            if ( CHADR % 4 )
            {
                /* уст.CHADR на гр.сл. */
                CHADR = (CHADR /4 + 1) * 4;
                /* запомн. в табл.симв. */
                T_SYM[ITSYM].ZNSYM = CHADR;
            }
            /* занулить PRNMET зн.'N'*/
            PRNMET = 'N';
        }
        else
        {
            /* иначе выход по ошибке  */
            return (1);
        }
    }
    /* если же псевдооп.непомеч */
    else
    {
        /* и CHADR не кратен 4,то: */
        if ( CHADR % 4 )
        {
            /* установ.CHADR на гр.сл.*/
            CHADR = (CHADR /4 + 1) * 4;
        }
    }
    /* увелич.CHADR на 4 и */
    CHADR = CHADR + 4;
    return (0);                                     /*успешно завершить подпр.*/
}

/* подпр.обр.пс.опер.END */
int FEND()
{
    /* выход с призн.конца 1-го просмотра */
    return (100);
}

/* подпр.обр.пс.опер.EQU */
int FEQU()
{
    /* если в операнде псевдооперации DC использован симв. '*' */
    if (TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0] == '*')
    {
        /* запомнить в табл.симв.: */
        /* CHADR в поле ZNSYM, */
        T_SYM[ITSYM].ZNSYM = CHADR;
        /* 1 в поле DLSYM, */
        T_SYM[ITSYM].DLSYM = 1;
        /* 'R' в пооле PRPER */
        T_SYM[ITSYM].PRPER = 'R';
    }
    /* иначе запомн.в табл.симв значение оп-нда пс.оп. DC в поле ZNSYM */
    else
    {
        T_SYM[ITSYM].ZNSYM = atoi ((char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND);
        T_SYM[ITSYM].DLSYM = 1;                      /* 1 в поле DLSYM,        */
        T_SYM[ITSYM].PRPER = 'A';                    /* 'A' в поле PRPER       */
    }
    PRNMET = 'N';                                   /*занул.PRNMET значен.'N' */
    return (0);                                     /*успешное заверш.подпр.  */
}

/* подпр.обр.пс.опер.START */
int FSTART()
{
    /*CHADR установить равным значению операнда псевдооперации START */
    CHADR = atoi((char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND);
    /*если это значение не кратно 8 */
    if ( CHADR % 8 )
    {
        /*, то сделать его кратным */
        CHADR = ( CHADR + ( 8 - CHADR % 8 ) );
    }
    /*запомнить в табл.симв.: */
    T_SYM[ITSYM].ZNSYM = CHADR;                     /* CHADR в поле ZNSYM,    */
    T_SYM[ITSYM].DLSYM = 1;                         /* 1 в поле DLSYM,        */
    T_SYM[ITSYM].PRPER = 'R';                       /* 'R' в поле PRPER       */
    PRNMET = 'N';                                   /*занул.PRNMET значен.'N' */
    return (0);                                     /*успешное заверш.подпрогр*/
}

/* подпр.обр.пс.опер.USING */
int FUSING()
{
    /* успешное заверш.подпрогр */
    return (0);
}

/* подпр.обр.опер.RR-форм. */
int FRR()
{
    /* увеличить сч.адр. на 2 */
    CHADR = CHADR + 2;
    /*если ранее обнар.метка, */
    if ( PRNMET == 'Y' )
    {
        /*то в табл. символов:    */
        T_SYM[ITSYM].DLSYM = 2;                       /*запомнить длину маш.опер*/
        T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
    }
    return(0);                                      /*выйти из подпрограммы   */
}

/* подпр.обр.опер.RX-форм. */
int FRX()
{
    /* увеличить сч.адр. на 4 */
    CHADR = CHADR + 4;
    /* если ранее обнар.метка, */
    if ( PRNMET == 'Y' )
    {
        /*то в табл. символов:    */
        T_SYM[ITSYM].DLSYM = 4;                       /*запомнить длину маш.опер*/
        T_SYM[ITSYM].PRPER = 'R';                     /*и установить призн.перем*/
    }
    return(0);                                      /*выйти из подпрограммы   */
}
/*..........................................................................*/

/*
******* Б Л О К  об'явлений подпрограмм, используемых при 2-ом просмотре
*/

/* подпр.формир.TXT-карты */
void STXT( int ARG )
{
    /* рабоч.переменная-указат. */
    char *PTR;

    /* формирование поля ADOP TXT-карты в формате двоичного целого в соглашениях ЕС ЭВМ */
    PTR = (char *)&CHADR;
    TXT.STR_TXT.ADOP[2]  = *PTR;
    TXT.STR_TXT.ADOP[1]  = *(PTR+1);
    TXT.STR_TXT.ADOP[0]  = '\x00';

    /* формирование поля OPER */
    if ( ARG == 2 )
    {
        /* для RR-формата */
        memset ( TXT.STR_TXT.OPER , 64 , 4 );
        memcpy ( TXT.STR_TXT.OPER,RR.BUF_OP_RR , 2 );
        TXT.STR_TXT.DLNOP [1] = 2;
    }
    else
    {
        /* для RX-формата */
        memcpy ( TXT.STR_TXT.OPER , RX.BUF_OP_RX , 4);
        TXT.STR_TXT.DLNOP [1] = 4;
    }
    /* формиров.идентифик.поля */
    memcpy (TXT.STR_TXT.POLE9,ESD.STR_ESD.POLE11,8);

    memcpy ( OBJTEXT[ITCARD] , TXT.BUF_TXT , 80 );  /*запись об'ектной карты  */
    ITCARD += 1;                                    /*коррекц.инд-са своб.к-ты*/
    CHADR = CHADR + ARG;                            /*коррекц.счетчика адреса */
    return;
}

/* подпр.обр.пс.опер.DC */
int SDC()
{
    /*рабочая переменная */
    char *RAB;

    /*занулим два старших байта RX.OP_RX */
    RX.OP_RX.OP = 0;
    RX.OP_RX.R1X2 = 0;

    /* если операнд начинается с комбинации F' */
    if (memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND,"F'", 2) == 0)
    {
        /* в перем. c указат.RAB выбираем первую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
        RAB=strtok((char*)TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND + 2, "'");

        /*перевод ASCII-> int */
        RX.OP_RX.B2D2 = atoi ( RAB );
        /* приведение к соглашениям ЕС ЭВМ */
        RAB = (char *) &RX.OP_RX.B2D2;
        swab ( RAB , RAB , 2 );
    }
    else
    {
        /* иначе сообщение об ошибке */
        return (1);
    }

    /* формирование TXT-карты */
    STXT (4);

    return (0);                                     /*успешн.завершение подпр.*/
}

/* подпр.обр.пс.опер.DS */
int SDS()
{
    /* занулим два старших байта RX.OP_RX */
    RX.OP_RX.OP   = 0;
    RX.OP_RX.R1X2 = 0;
    /* если операнд начинается с комбинации F' */
    if (TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND[0]=='F')
    {
        /* занулим RX.OP_RX.B2D2 */
        RX.OP_RX.B2D2 = 0;
    }
    else
    {
        /* сообщение об ошибке */
        return (1);
    }

    /* формирование TXT-карты */
    STXT (4);

    return (0);                                     /*успешно завершить подпр.*/
}

/* подпр.обр.пс.опер.END */
int SEND()
{
    /* формирование идентификационнго поля END - карты */
    memcpy(END.STR_END.POLE9, ESD.STR_ESD.POLE11, 8);
    /*запись об'ектной карты в массив об'ектных карт*/
    memcpy(OBJTEXT[ITCARD], END.BUF_END, 80);
    /* коррекц.инд-са своб. к-ты */
    ITCARD += 1;
    /* выход с призн.конца 2-го просмотра */
    return (100);
}

/* подпр.обр.пс.опер.EQU */
int SEQU()
{
    /*успешное заверш.подпр.  */
    return (0);
}

/* подпр.обр.пс.опер.START */
int SSTART()
{
    /* набор рабочих переменных подпрограммы */
    char *PTR;
    char *METKA;
    char *METKA1;
    int J;
    int RAB;

    /* в перем. c указат.METKA1 выбираем первую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
    METKA1 = strtok((char*) TEK_ISX_KARTA.STRUCT_BUFCARD.METKA, " ");

    for ( J=0; J<=ITSYM; J++ )
    {
        /* все метки исх.текста в табл. T_SYM сравниваем со знач.перем. *METKA1 */
        METKA = strtok ((char*) T_SYM[J].IMSYM , " ");

        /* и при совпадении:      */
        if( !strcmp ( METKA , METKA1 ) )
        {
            /*  берем разность сч.адр. знач.этой метки, образуя длину программы в соглашениях ЕС ЭБМ, и записыв.ее в ESD-карту побайтно */
            RAB = CHADR - T_SYM[J].ZNSYM;
            PTR = (char *)&RAB;
            swab ( PTR , PTR , 2 );
            ESD.STR_ESD.DLPRG [0] = 0;
            ESD.STR_ESD.DLPRG [1] = *PTR;
            ESD.STR_ESD.DLPRG [2] = *(PTR + 1);

            /* устанавл.CHADR, равным операнду операт.START исходного текста */
            CHADR = T_SYM[J].ZNSYM;

            /* формирование поля ADPRG ESD-карты в формате двоичного целого в соглашениях ЕС ЭВМ */
            PTR = (char *)&CHADR;
            ESD.STR_ESD.ADPRG[2]  = *PTR;
            ESD.STR_ESD.ADPRG[1]  = *(PTR+1);
            ESD.STR_ESD.ADPRG[0]  = '\x00';

            /* формирование имени программы и идентификационнго поля ESD - карты */
            memcpy(ESD.STR_ESD.IMPR, METKA, strlen(METKA));
            memcpy(ESD.STR_ESD.POLE11, METKA, strlen(METKA));

            /* запись об'ектной карты в массив об'ектных карт */
            memcpy(OBJTEXT[ITCARD], ESD.BUF_ESD, 80);
            ITCARD += 1;                                /*коррекц.инд-са своб.к-ты*/
            return (0);                                 /*успешное заверш.подпрогр*/
        }
    }
    return (2);                                     /*ошибочное заверш.прогр. */
}

/* подпр.обр.пс.опер.USING */
int SUSING()
{
    /* набор рабочих переменных */
    char *METKA;
    char *METKA1;
    char *METKA2;
    int J;
    int NBASRG;

    /* в перем. c указат.METKA1 выбираем первую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
    METKA1 = strtok((char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, ",");

    /* в перем. c указат.METKA2 выбираем вторую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
    METKA2 = strtok(NULL, " ");

    /*если лексема начинается с буквы, то: */
    if (isalpha ((int)*METKA2))
    {
        /* все метки исх.текста в табл. T_SYM сравниваем */
        for ( J=0; J<=ITSYM; J++ )
        {
            /* со знач.перем. *METKA2 */
            METKA = strtok ((char*) T_SYM[J].IMSYM , " ");
            /* и при совпадении:      */
            if( !strcmp ( METKA , METKA2 ) )
            {
                /*  запоминаем значение метки в NBASRG и в сл. NBASRG <= 0x0f идем на устан.регистра базы */
                if ( (NBASRG = T_SYM[J].ZNSYM) <= 0x0f )
                {
                    goto SUSING1;
                }
                else
                {
                    /* сообщение об ошибке */
                    return (6);
                }
            }
        }
        /*заверш.подпр.по ошибке  */
        return (2);
    }
    /*иначе, если второй опер. начинается с цифры, то: */
    else
    {
        /* запомним его в NBASRG */
        NBASRG = atoi ( METKA2 );
        /* и,если он <= 0x0f,то: */
        if ( (NBASRG = T_SYM[J].ZNSYM) <= 0x0f )
        {
            /* идем на устан.рег.базы */
            goto SUSING1;
        }
        else
        {
            /*иначе: сообщение об ошибке */
            return (6);
        }
    }

SUSING1:
    /*установить базовый рег. */
    /* взвести призн.активн. */
    T_BASR [NBASRG - 1].PRDOST = 'Y';

    /* если перв.оп-нд == '*',*/
    if ( *METKA1 == '*' )
    {
        /* выбир.знач.базы ==CHADR */
        T_BASR[NBASRG-1].SMESH = CHADR;
    }
    else
    {
        /* все метки исх.текста в табл. T_SYM */
        for ( J=0; J<=ITSYM; J++ )
        {
            /* сравниваем со знач.перем. *METKA1 */
            METKA = strtok ((char*) T_SYM[J].IMSYM , " ");
            /* и при совпадении: */
            if( !strcmp ( METKA , METKA1 ) )
            {
                /*  берем значение этой этой метки как базу */
                T_BASR[NBASRG-1].SMESH = T_SYM[J].ZNSYM;
            }
        }
        /* завершение прогр.по ошиб */
        return (2);
    }
    /* успешное заверш.подпрогр */
    return (0);
}

/* подпр.обр.опер.RR-форм. */
int SRR()
{
    /*набор рабочих переменных */
    char *METKA;
    char *METKA1;
    char *METKA2;
    unsigned char R1R2;
    int J;
    /* формирование кода операц */
    RR.OP_RR.OP = T_MOP[I3].CODOP;

    /* в перем. c указат.METKA1 выбираем первую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
    METKA1 = strtok((char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, ",");

    /* в перем. c указат.METKA2 выбираем вторую лексему операнда текущей карты исх.текста АССЕМБЛЕРА */
    METKA2 = strtok(NULL, " ");

    /* если лексема начинается с буквы, то: */
    if ( isalpha ( (int) *METKA1 ) )
    {
        /* все метки исх.текста в табл. T_SYM */
        for ( J=0; J<=ITSYM; J++ )
        {
            /* сравниваем со знач.перем. *METKA1 */
            METKA = strtok ((char*) T_SYM[J].IMSYM , " ");

            /* и при совпадении: */
            if( !strcmp ( METKA , METKA1 ) )
            {
                /*  берем значение этой метки в качестве перв. опреранда машинной ком */
                R1R2 = T_SYM[J].ZNSYM << 4;
                goto SRR1;
            }
        }
        /* сообщ."необ'явл.идентиф" */
        return(2);
    }
    /*иначе,*/
    else
    {
        /* берем в качестве перв.операнда машинн.ком значен.выбр. лексемы */
        R1R2 = atoi ( METKA1 ) << 4;
    }

SRR1:

    /* если лексема начинается с буквы, то: */
    if ( isalpha ( (int) *METKA2 ) )
    {
        /* все метки исх.текста в табл. T_SYM */
        for ( J=0; J<=ITSYM; J++ )
        {
            /*  сравниваем со знач.перем. *МЕТКА2 */
            METKA = strtok ((char*) T_SYM[J].IMSYM , " ");
            /* и при совпадении: */
            if( !strcmp ( METKA , METKA2 ) )
            {
                /*  берем значение этой метки в качестве втор. опреранда машинной ком */
                R1R2 = R1R2 + T_SYM[J].ZNSYM;
                goto SRR2;
            }
        }
        /* сообщ."необ'явл.идентиф" */
        return(2);
    }
    /* иначе */
    else
    {
        /* берем в качестве втор.операнда машинн.ком значен.выбр. лексемы */
        R1R2 = R1R2 + atoi ( METKA2 );
    }

SRR2:

    /* формируем опер-ды маш-ой команды */
    RR.OP_RR.R1R2 = R1R2;

    STXT(2);

    /*выйти из подпрограммы   */
    return(0);
}

/*..........................................................................*/
int SRX()                                         /*подпр.обр.опер.RX-форм. */
 {
  char *METKA;                                    /*набор                   */
  char *METKA1;                                   /*рабочих                 */
  char *METKA2;                                   /*переменных              */
  char *PTR;                                      /*                        */
  int  DELTA;                                     /*                        */
  int  ZNSYM;                                     /*                        */
  int  NBASRG;                                    /*                        */
  int J;                                          /*                        */
  int I;                                          /*                        */
  unsigned char R1X2;                             /*                        */
  int B2D2;                                       /*                        */
  RX.OP_RX.OP = T_MOP[I3].CODOP;                  /*формирование кода операц*/
  METKA1 = strtok                                 /*в перем. c указат.METKA1*/
	   (                                      /*выбираем первую лексему */
    (char*) TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAND, /*операнда текущей карты  */
	    ","                                   /*исх.текста АССЕМБЛЕРА   */
	   );

  METKA2 = strtok                                 /*в перем. c указат.METKA2*/
	   (                                      /*выбираем вторую лексему */
	    NULL,                                 /*операнда текущей карты  */
	    " "                                   /*исх.текста АССЕМБЛЕРА   */
	   );

  if ( isalpha ( (int) *METKA1 ) )                /*если лексема начинается */
   {                                              /*с буквы, то:            */
    for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
     {                                            /* табл. T_SYM сравниваем */
						  /* со знач.перем. *METKA  */
      METKA = strtok (
		      (char*) T_SYM[J].IMSYM , " "
		     );
      if( !strcmp ( METKA , METKA1 ) )            /* и при совпадении:      */

       {                                          /*  берем значение этой   */
	 R1X2 = T_SYM[J].ZNSYM << 4;              /*  метки в качестве перв.*/
	 goto SRX1;
       }                                          /*  опреранда машинной ком*/
     }
    return(2);                                    /*сообщ."необ'явл.идентиф"*/
   }
  else                                            /*иначе, берем в качестве */
   {                                              /*перв.операнда машинн.ком*/
     R1X2 = atoi ( METKA1 ) << 4;                 /*значен.выбр.   лексемы  */
   }


 SRX1:


  if ( isalpha ( (int) *METKA2 ) )                /*если лексема начинается */
   {                                              /*с буквы, то:            */
    for ( J=0; J<=ITSYM; J++ )                    /* все метки исх.текста в */
     {                                            /* табл. T_SYM сравниваем */
						  /* со знач.перем. *МЕТКА  */
      METKA = strtok (
		      (char*) T_SYM[J].IMSYM , " "
		     );
      if( !strcmp ( METKA , METKA2 ) )            /* и при совпадении:      */
       {                                          /*  установить нач.знач.: */
	NBASRG = 0;                               /*   номера базов.регистра*/
	DELTA  = 0xfff - 1;                       /*   и его значен.,а также*/
	ZNSYM  = T_SYM[J].ZNSYM;                  /*   смещен.втор.операнда */
	for ( I=0; I<15; I++ )                    /*далее в цикле из всех   */
	 {                                        /*рег-ров выберем базовым */
	  if (                                    /*тот, который имеет:     */
	       T_BASR[I].PRDOST == 'Y'            /* призн.активности,      */
	      &&                                  /*  и                     */
	       ZNSYM - T_BASR[I].SMESH >= 0       /* значенение, меньшее по */
	      &&                                  /* величине,но наиболее   */
	       ZNSYM - T_BASR[I].SMESH < DELTA    /* близкое к смещению вто-*/
	     )                                    /* рого операнда          */
	   {
	    NBASRG = I + 1;
	    DELTA  = ZNSYM - T_BASR[I].SMESH;
	   }
	 }
	if ( NBASRG == 0 || DELTA > 0xfff )       /*если баз.рег.не выбр.,то*/
	 return(5);                               /* заверш.подпр.по ошибке */
	else                                      /*иначе                   */
	 {                                        /* сформировыать машинное */
	  B2D2 = NBASRG << 12;                    /* представление второго  */
	  B2D2 = B2D2 + DELTA;                    /* операнда в виде B2D2   */
	  PTR = (char *)&B2D2;                    /* и в соглашениях ЕС ЭВМ */
	  swab ( PTR , PTR , 2 );                 /* с записью в тело ком-ды*/
	  RX.OP_RX.B2D2 = B2D2;
	 }
	goto SRX2;                                /*перех.на форм.первого   */
       }                                          /*  опреранда машинной ком*/
     }
    return(2);                                    /*сообщ."необ'явл.идентиф"*/
   }
  else                                            /*иначе, берем в качестве */
   {                                              /*втор.операнда машинн.ком*/
    return(4);                                    /*значен.выбр.   лексемы  */
   }

 SRX2:

  RX.OP_RX.R1X2 = R1X2;                           /*дозапись перв.операнда  */

  STXT(4);                                        /*формирование TXT-карты  */
  return(0);                                      /*выйти из подпрограммы   */
 }
/*..........................................................................*/
int SOBJFILE()                                    /*подпрогр.формир.об'екн. */
 {                                                /*файла                   */
  FILE *fp;                                       /*набор рабочих           */
  int RAB2;                                       /*переменных              */
						  /*формирование пути и име-*/
  strcat ( NFIL , "tex" );                        /*ни об'ектного файла     */
  if ( (fp = fopen ( NFIL , "wb" )) == NULL )     /*при неудачн.открыт.ф-ла */
   return (-7);                                   /* сообщение об ошибке    */
  else                                            /*иначе:                  */
   RAB2 =fwrite (OBJTEXT, 80 , ITCARD , fp);      /* формируем тело об.файла*/
  fclose ( fp );                                  /*закрываем об'ектный файл*/
  return ( RAB2 );                                /*завершаем  подпрограмму */

 }
/*..........................................................................*/
void INITUNION ()
{

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей ESD-типа
    *****                             для выходного объектного файла
    */

    ESD.STR_ESD.POLE1      =  0x02;
    memcpy ( ESD.STR_ESD.POLE2, "ESD", 3 );
    memset ( ESD.STR_ESD.POLE3,  0x40, 6 );
    ESD.STR_ESD.POLE31 [0] = 0x00;
    ESD.STR_ESD.POLE31 [1] = 0x10;
    memset ( ESD.STR_ESD.POLE32, 0x40, 2 );
    ESD.STR_ESD.POLE4  [0] = 0x00;
    ESD.STR_ESD.POLE4  [1] = 0x01;
    memset ( ESD.STR_ESD.IMPR,   0x40, 8 );
    ESD.STR_ESD.POLE6      = 0x00;
    memset ( ESD.STR_ESD.ADPRG,  0x00, 3 );
    ESD.STR_ESD.POLE8      = 0x40;
    memset ( ESD.STR_ESD.DLPRG,  0x00, 3 );
    memset ( ESD.STR_ESD.POLE10, 0x40,40 );
    memset ( ESD.STR_ESD.POLE11, 0x40, 8 );

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей TXT-типа
    *****                             для выходного объектного файла
    */

    TXT.STR_TXT.POLE1      =  0x02;
    memcpy ( TXT.STR_TXT.POLE2, "TXT", 3 );
    TXT.STR_TXT.POLE3      =  0x40;
    memset ( TXT.STR_TXT.ADOP,   0x00, 3 );
    memset ( TXT.STR_TXT.POLE5,  0x40, 2 );
    memset ( TXT.STR_TXT.DLNOP,  0X00, 2 );
    memset ( TXT.STR_TXT.POLE7,  0x40, 2 );
    TXT.STR_TXT.POLE71 [0] = 0x00;
    TXT.STR_TXT.POLE71 [1] = 0x01;
    memset ( TXT.STR_TXT.OPER,   0x40,56 );
    memset ( TXT.STR_TXT.POLE9,  0x40, 8 );

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей END-типа
    *****                             для выходного объектного файла
    */

    END.STR_END.POLE1      =  0x02;
    memcpy ( END.STR_END.POLE2, "END", 3 );
    memset ( END.STR_END.POLE3,  0x40,68 );
    memset ( END.STR_END.POLE9,  0x40, 8 );
}

/*..........................................................................*/

int main( int argc, char **argv )                /*главная программа       */
{
    FILE *fp;
    char *ptr = argv [1];
    unsigned char ASSTEXT [DL_ASSTEXT][80];

    /*
    ******* Б Л О К  об'явлений рабочих переменных
    */

    int I1 , I2 , RAB;                              /* переменные цикла      */

    /* начальное заполнение буферов формирования записей выходного объектного файла */
    INITUNION ();

    /*
    ******       Н А Ч А Л О   П Е Р В О Г О  П Р О С М О Т Р А      *****
    */
    /*
    ***** Б Л О К  инициализации массива ASSTEXT, заменяющий иниц-ю в об'явлении
    *****          (введен как реакция на требования BORLANDC++ 2.0)
    */

    strcpy ( NFIL, ptr );

    if ( argc != 2 )
    {
        printf("Ошибка в командной строке\n");
        return -1;
    }

    if ( strcmp(&NFIL[strlen(NFIL) - 3], "ass") != 0)
    {
        printf("Неверный тип файла с исходным текстом\n");
        return -1;
    }

    fp = fopen ( NFIL , "rb");
    if (fp == NULL)
    {
        printf("Не найден файл с исходным текстом\n");
        return -1;
    }

    for ( I1 = 0; I1 <= DL_ASSTEXT; I1++ )
    {
        if ( !fread ( ASSTEXT [I1], 80, 1, fp ) )
        {
            if (feof(fp))
            {
                goto main1;
            }
            else
            {
                printf("Ошибка при чтении фыйла с исх.текстом\n");
                fclose ( fp );
                return -1;
            }
        }
    }

    printf ( "%s\n", "Переполнение буфера чтения исх.текста" );
    fclose ( fp );
    return -1;

main1:

    NFIL [ strlen ( NFIL )-3 ] = '\x0';

    /*
    ***** К О Н Е Ц блока инициализации
    */

    /* для карт с 1 по конечную ч-ть очередн.карту в буф */
    for ( I1=0; I1 < DL_ASSTEXT; I1++ )
    {
        memcpy ( TEK_ISX_KARTA.BUFCARD , ASSTEXT[I1], 80);

        /* переход при отсутствии метки на CONT1 */
        if (TEK_ISX_KARTA.STRUCT_BUFCARD.METKA [0] == ' ')
        {
            goto CONT1;
        }

        /*иначе: переход к след.стр.TSYM устан.призн.налич.метки запомнить имя символа и его значение(отн.адр.) */
        ITSYM += 1;
        PRNMET = 'Y';
        memcpy ( T_SYM[ITSYM].IMSYM , TEK_ISX_KARTA.STRUCT_BUFCARD.METKA , 8 );
        T_SYM[ITSYM].ZNSYM = CHADR;

        /*
        ***** Б Л О К  поиска текущей операции среди псевдоопераций
        */

CONT1:

        /* для всех стр. таб. пс. опер выполнить следущее: */
        for ( I2=0; I2 < NPOP; I2++ )
        {
            /* если псевдооперация распознана, то: */
            if (memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_POP[I2].MNCPOP , 5) == 0)
            {
                /* уйти в подпр.обработки */
                switch ( T_POP[I2].BXPROG () )
                {
                    case 0:
                    {
                        /* и завершить цикл */
                        goto CONT2;
                    }
                    case 1:
                    {
                        goto ERR1;
                    }
                    case 100:
                    {
                        goto CONT3;
                    }
                }
            }
        }

        /* для всех стр. таб. м. опер. выполнить следующее: */
        for ( I3=0; I3 < NOP ; I3++ )
        {
            /* если машинная операция распознана, то: */
            if (memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_MOP[I3].MNCOP, 5) == 0)
            {
                dbg("OPERAC[0] = %c", TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC[0]);
                dbg("OPERAC[1] = %c", TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC[1]);
                dbg("OPERAC[2] = %c", TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC[2]);
                dbg("OPERAC[3] = %c", TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC[3]);
                dbg("OPERAC[4] = %c", TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC[4]);
                T_MOP[I3].BXPROG ();                     /* уйти в подпр.обработки */
                PRNMET = 'N';                            /* снять призн.налич.метки*/
                goto CONT2;                              /* и завершить цикл       */
            }
        }

        /*сообщ.'мнемокод нерасп.'*/
        goto ERR3;

CONT2:
        continue;

        /*конец цикла обработки карт исх.текста */
    }


    /*
    ******       Н А Ч А Л О   В Т О Р О Г О  П Р О С М О Т Р А      *****
    */

CONT3:

    /*установить указатели на подпрограммы обраб-ки команд АССЕМБЛЕРА при втором просмотре */
    T_MOP[0].BXPROG = SRR;
    T_MOP[1].BXPROG = SRR;
    T_MOP[2].BXPROG = SRX;
    T_MOP[3].BXPROG = SRX;
    T_MOP[4].BXPROG = SRX;
    T_MOP[5].BXPROG = SRX;
    T_MOP[6].BXPROG = SRX;

    /*установить указатели на подпрограммы обраб-ки псевдокоманд АССЕМБЛЕРА при втором просмотре */
    T_POP[0].BXPROG = SDC;
    T_POP[1].BXPROG = SDS;
    T_POP[2].BXPROG = SEND;
    T_POP[3].BXPROG = SEQU;
    T_POP[4].BXPROG = SSTART;
    T_POP[5].BXPROG = SUSING;

    /*для карт с 1 по конечную*/
    for ( I1=0; I1 < DL_ASSTEXT; I1++ )
    {
        /* ч-ть очередн.карту в буф */
        memcpy ( TEK_ISX_KARTA.BUFCARD , ASSTEXT [I1], 80);

        /*
        ***** Б Л О К  поиска текущей операции среди псевдоопераций
        */

        /*для всех стр. таб. пс. опер выполнить следущее: */
        for ( I2=0; I2 < NPOP; I2++ )
        {
            /* если псевдооперация распознана, то */
            if (memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_POP[I2].MNCPOP , 5) == 0)
            {
                /* уйти в подпр.обработки */
                switch ( T_POP[I2].BXPROG () )
                {
                    case 0:
                    {
                        /* и завершить цикл */
                        goto CONT4;
                    }

                    case 100:
                    {
                        /* уйти на формирование об'ектного файла */
                        goto CONT5;
                    }
                }
            }
        }

        /* для всех стр. таб. м. опер. выполнить следующее: */
        for ( I3=0; I3 < NOP ; I3++ )
        {
            /* если машинная операция распознана то */
            if (memcmp(TEK_ISX_KARTA.STRUCT_BUFCARD.OPERAC, T_MOP[I3].MNCOP , 5) == 0)
            {
                /* уйти в подпр.обработки */
                switch ( T_MOP[I3].BXPROG () )
                {
                    case 0:
                    {
                        /* и завершить цикл */
                        goto CONT4;
                    }

                    case 2:                                /*выдать диагностическое  */
                        goto ERR2;                            /*сообщение               */

                    case 4:                                /*выдать диагностическое  */
                        goto ERR4;                            /*сообщение               */

                    case 5:                                /*выдать диагностическое  */
                        goto ERR5;                            /*сообщение               */

                    case 6:                                /*выдать диагностическое  */
                        goto ERR6;                            /*сообщение               */

                    case 7:                                /*выдать диагностическое  */
                        goto ERR6;                            /*сообщение               */
                }
            }
        }

CONT4:
        continue;
        /*конец цикла обработки карт исх.текста */
    }

CONT5:

    RAB = SOBJFILE ();
    /* если в об'ектный файл выведены все карты, то: */
    if ( ITCARD == RAB )
    {
        printf("успешое завершение трансляции\n");
        return 0;
    }
    else
    {
        if ( RAB == -7 )
        {
            goto ERR7;
        }
        else
        {
            printf("ошибка при формировании об'ектного файла\n");
        }
    }
    return -1;

ERR1:
    printf ("%s\n","ошибка формата данных");        /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR2:
    printf ("%s\n","необ'явленный идентификатор");  /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR3:
    printf ("%s\n","ошибка кода операции");         /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR4:
    printf ("%s\n","ошибка второго операнда");      /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR5:
    printf ("%s\n","ошибка базирования");           /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR6:
    printf ("%s\n","недопустимый номер регистра");  /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR7:
    printf ("%s\n","ошибка открытия об'ектн.файла");/*выдать диагностич.сообщ.*/
    goto CONT6;


CONT6:
    printf ("%s%d\n","ошибка в карте N ",I1+1);     /*выдать диагностич.сообщ.*/

    return 0;
}

