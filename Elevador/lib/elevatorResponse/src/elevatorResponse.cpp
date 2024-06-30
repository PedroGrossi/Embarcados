/*#######################################################################
Aluno = Pedro Henrique Grossi da Silva
Data = 29/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando Ambiente ARDUINO
#######################################################################*/
/* Biblioteca do Arduino */
#include <Arduino.h>

/* Libs desenvolvidas*/
#include "..\..\..\src\main.h"

/* Protótipo das funções*/
void initialized(char *rx, char *tx, struct elevador *esquerdo);
void doorStatus(char *rx, char *tx, struct elevador *esquerdo);
void floorVerify(char *rx, char *tx, char n_btc_in, char n_seq_up, unsigned long timerelevador, struct elevador *esquerdo);
int cabinButton(char *rx, char*tx, char n_btc_in, int floorTarget, struct elevador *esquerdo);
int hallwayUpButton(char *rx, char *tx, char n_seq_up, int floorTarget, struct elevador *esquerdo);
int hallwayDownButton(char *rx, char *tx, char n_seq_down, int floorTarget, struct elevador *esquerdo);

void initialized(char *rx, char *tx, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg de inicialização:
    if (rx[0]=='i'&&rx[1]=='n'&&rx[2]=='i'&&rx[3]=='t'&&rx[4]=='i'&&rx[5]=='a'&&rx[6]=='l'&&rx[7]=='i'&&rx[8]=='z'&&rx[9]=='e'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        //contador genérico:
        int i;
        //Envia msg resete elevador esquerdo - delay para esperar o SIMSE2 abrir:
        delay(1000);//delay para SIMSE2 abrir: varia de PC para PC (1000-4000). SUPER Loop: Sem delay no programa principal ... aqui nao afeta o funcionamento apos init ...
        tx[0]='e';tx[1]='r';tx[2]='\r';
        Serial.write(tx,3);
        esquerdo->andar=0;
        esquerdo->estado='P';
        esquerdo->prox=0;
        esquerdo->n_btc_in=0;
        esquerdo->n_seq_up=0;
        esquerdo->n_seq_down=0;
        for (i=15;i>=0;i--) esquerdo->btc_in[i]=0;
        for (i=15;i>=0;i--) esquerdo->btc_down[i]=0;
        for (i=15;i>=0;i--) esquerdo->btc_up[i]=0;
        for (i=15;i>=0;i--) esquerdo->seq_up[i]=0;
        for (i=15;i>=0;i--) esquerdo->seq_down[i]=0;
        zerar_serial();
    }
}

void doorStatus(char *rx, char *tx, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg porta aberta:
    if (rx[9]=='e'&&rx[10]=='A'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->porta=0;
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg porta fechada:
    if (rx[9]=='e'&&rx[10]=='F'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->estado=='S')
        {
        tx[0]='e';tx[1]='s';tx[2]='\r';
        Serial.write(tx,3);
        }
        if (esquerdo->estado=='D')
        {
        tx[0]='e';tx[1]='d';tx[2]='\r';
        Serial.write(tx,3);
        }
        esquerdo->porta=1;
        zerar_serial();
    }
}

void floorVerify(char *rx, char *tx, char n_btc_in, char n_seq_up, unsigned long timerelevador, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg elevador no terreo:
    if (rx[9]=='e'&&rx[10]=='0'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=0;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[0]=0;
            esquerdo->btc_up[0]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='a';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='a';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 1 andar:
    if (rx[9]=='e'&&rx[10]=='1'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=1;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[1]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[1]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[0]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='b';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='b';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 2 andar:
    if (rx[9]=='e'&&rx[10]=='2'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=2;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[2]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[2]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[1]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='c';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='c';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 3 andar:
    if (rx[9]=='e'&&rx[10]=='3'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=3;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[3]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[3]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[2]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='d';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='d';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 4 andar:
    if (rx[9]=='e'&&rx[10]=='4'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=4;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[4]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[4]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[3]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            tx[0]='e';tx[1]='D';tx[2]='e';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='e';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 5 andar:
    if (rx[9]=='e'&&rx[10]=='5'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=5;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[5]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[5]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[4]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='f';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='f';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 6 andar:
    if (rx[9]=='e'&&rx[10]=='6'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=6;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[6]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[6]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[5]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='g';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='g';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 7 andar:
    if (rx[9]=='e'&&rx[10]=='7'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=7;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[7]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[7]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[6]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='h';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='h';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 8 andar:
    if (rx[9]=='e'&&rx[10]=='8'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=8;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[8]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[8]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[7]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='i';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='i';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 9 andar:
    if (rx[9]=='e'&&rx[10]=='9'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=9;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[9]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[9]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[8]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='j';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='j';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 10 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='0'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=10;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[10]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[10]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[9]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='k';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='k';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 11 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='1'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=11;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[11]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[11]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[10]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='l';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='l';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 12 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='2'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=12;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[12]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[12]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[11]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='m';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='m';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 13 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='3'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=13;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[13]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[13]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[12]=0;
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='n';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='n';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 14 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='4'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=14;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[14]=0;
            if (esquerdo->estado=='S') esquerdo->btc_up[14]=0;
            if (esquerdo->estado=='D') esquerdo->btc_down[13]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='o';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
            tx[0]='c';tx[1]='D';tx[2]='o';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg elevador no 15 andar:
    if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='5'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        esquerdo->andar=15;
        if (esquerdo->andar==esquerdo->prox)
        {
            esquerdo->btc_in[15]=0;
            esquerdo->btc_down[14]=0;
            timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
            tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
            esquerdo->estado='P';                                                  //estado=parado
            tx[0]='e';tx[1]='D';tx[2]='p';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
            tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
            n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            //Avisa que o elevaor chegou ao andar
        }
       zerar_serial();
    }
};

int cabinButton(char *rx, char*tx, char n_btc_in, int floorTarget, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine terreo 0:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='a'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=0)
        {
            esquerdo->btc_in[0]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='a';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=0;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 1 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='b'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=1)
        {
            esquerdo->btc_in[1]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='b';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=1;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 2 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='c'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=2)
        {
            esquerdo->btc_in[2]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='c';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=2;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 3 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=3)
        {
            esquerdo->btc_in[3]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='d';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=3;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 4 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='e'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=4)
        {
            esquerdo->btc_in[4]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='e';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=4;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 5 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='f'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=5)
        {
            esquerdo->btc_in[5]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='f';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=5;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 6 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='g'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=6)
        {
            esquerdo->btc_in[6]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='g';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=6;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 7 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='h'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=7)
        {
            esquerdo->btc_in[7]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='h';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=7;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 8 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='i'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=8)
        {
            esquerdo->btc_in[8]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='i';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=8;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 9 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='j'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=9)
        {
            esquerdo->btc_in[9]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='j';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=9;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 10 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='k'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=10)
        {
            esquerdo->btc_in[10]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='k';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=10;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 11 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='l'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=11)
        {
            esquerdo->btc_in[11]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='l';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=11;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 12 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='m'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=12)
        {
            esquerdo->btc_in[12]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='m';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=12;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 13 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='n'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=13)
        {
            esquerdo->btc_in[13]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='n';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=13;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 14 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='o'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=14)
        {
            esquerdo->btc_in[14]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='o';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=14;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o Interno da Cabine 15 andar:
    if (rx[8]=='e'&&rx[9]=='I'&&rx[10]=='p'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=15)
        {
            esquerdo->btc_in[15]=1;
            //Envia msg ascender luz do botÃ£o:
            tx[0]='e';tx[1]='L';tx[2]='p';tx[3]='\r'; Serial.write(tx,4);
            //Coloca o andar na fila
            floorTarget=15;
            n_btc_in=0;                                                            //reset contadores de eventos ...
        }
        zerar_serial();
    }

    return floorTarget;
}

int hallwayUpButton(char *rx, char *tx, char n_seq_up, int floorTarget, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg BotÃ£o corredor terreo sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='0'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=0) esquerdo->btc_up[0]=1;                              //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='a';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=0;

        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 1 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='1'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=1) esquerdo->btc_up[1]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='b';tx[3]='\r';Serial.write(tx,4);
        
        //Colocar o andar na fila
        floorTarget=1;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 2 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='2'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=2) esquerdo->btc_up[2]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='c';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=2;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 3 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='3'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=3) esquerdo->btc_up[3]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='d';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=3;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 4 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='4'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=4) esquerdo->btc_up[4]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='e';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=4;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 5 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='5'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=5) esquerdo->btc_up[5]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='f';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=5;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 6 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='6'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=6) esquerdo->btc_up[6]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='g';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=6;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 7 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='7'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=7) esquerdo->btc_up[7]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='h';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=7;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 8 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='8'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=8) esquerdo->btc_up[8]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='i';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=8;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 9 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='9'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=9) esquerdo->btc_up[9]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='j';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=9;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 10 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='0'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=10) esquerdo->btc_up[10]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='k';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=10;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 11 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='1'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=11) esquerdo->btc_up[11]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='l';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=11;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 12 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='2'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=12) esquerdo->btc_up[12]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='m';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=12;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 13 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='3'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=13) esquerdo->btc_up[13]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='n';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=13;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 14 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='4'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=14) esquerdo->btc_up[14]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='c';tx[1]='L';tx[2]='o';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=14;
        
        n_seq_up=0;                                                               //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 15 andar sobe: nÃ£o existe
    return floorTarget;
}

int hallwayDownButton(char *rx, char *tx, char n_seq_down, int floorTarget, struct elevador *esquerdo)
{
    //SIMSE2 -> Enviando msg BotÃ£o corredor terreo desce: nÃ£o existe
    //SIMSE2 -> Enviando msg BotÃ£o corredor 1 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='1'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=1) esquerdo->btc_down[0]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='b';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=1;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 2 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='2'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=2) esquerdo->btc_down[1]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='c';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=2;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 3 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='3'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=3) esquerdo->btc_down[2]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='d';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=3;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 4 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='4'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=4) esquerdo->btc_down[3]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='e';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=4;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 5 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='5'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=5) esquerdo->btc_down[4]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='f';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=5;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 6 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='6'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=6) esquerdo->btc_down[5]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='g';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=6;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 7 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='7'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=7) esquerdo->btc_down[6]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='h';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=7;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 8 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='8'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=8) esquerdo->btc_down[7]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='i';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=8;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 9 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='9'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=9) esquerdo->btc_down[8]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='j';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=9;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 10 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='0'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=10) esquerdo->btc_down[9]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='k';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=10;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 11 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='1'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=11) esquerdo->btc_down[10]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='l';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=11;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 12 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='2'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=12) esquerdo->btc_down[11]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='m';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=12;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 13 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='3'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=13) esquerdo->btc_down[12]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='n';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=13;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 14 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='4'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=14) esquerdo->btc_down[13]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='o';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=14;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 15 andar desce:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='1'&&rx[9]=='5'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
        if (esquerdo->andar!=15) esquerdo->btc_down[14]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
        
        //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
        tx[0]='d';tx[1]='L';tx[2]='p';tx[3]='\r';Serial.write(tx,4);

        //Colocar o andar na fila
        floorTarget=15;
        
        n_seq_down=0;                                                             //reset contadores de eventos ...
        zerar_serial();
    }
    return floorTarget;
}
