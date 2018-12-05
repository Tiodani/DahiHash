/*
 * File:   mainBMK.cpp
 * Author: Daniel Franklin
 *
 * Created on 2018
 * Part of the DaniHash project
 */

#include <fstream>
#include "../DaniHash_headers/utilOutput.h"
#include "../DaniHash_headers/utilMath.h"
#include "../DaniHash_headers/utilInput.h"
#include "../DaniHash_headers/TabelaHash.h"

using namespace std;
using namespace dh;

int main(){

    Atributos atributos;
    bool loopPrograma = true, loopMenu;
    int opcaoMenu, opcaoTmp, numeroTmp, codigoBmkTmp;
    string comandoTmp, fileNameTmp;

    /** Variaveis usadas nos benchmarks **/
    int rehashingsTotais;
    unsigned int MAXcolisoes;
    double tempoTotal_insercoes, tempoTotal_buscas, tempoDecorrido, qtdInsercoes;
    Results resultadoBMK;
    /*************************************/

    while (loopPrograma){

        loopMenu = true;
        system("cls");
        cout<<"***DANIHASH BMK v1.0.3***\n";
        atributos = pegaAtributosDaHash(false);

        if (atributos.tipo == 0){   //Para sair dos loops e encerrar o programa
            loopMenu = false;
            loopPrograma = false;
        }else{
            loopMenu = true;
            loopPrograma = true;
        }

        TabelaHash* h;

        while (loopMenu){

            system("cls");

            cout<<"O que deseja?\n\n";
            cout<<"1) Ler e inserir numeros de um arquivo\n";
            cout<<"2) Inserir numeros ordenados\n";
            cout<<"3) Inserir numeros aleatorios (Uniforme)\n";
            cout<<"4) Inserir numeros aleatorios (Normal)\n";
            cout<<"5) Inserir numeros aleatorios (Exponencial)\n";
            cout<<"6) Realizar benchmark completo (Uniforme)\n";
            cout<<"7) Realizar benchmark completo (Normal)\n";
            cout<<"8) Realizar benchmark completo (Exponencial)\n";
            cout<<"9) Retornar ao menu inicial\n\n";
            opcaoMenu = input::pegaRespostaMinMax("Opcao: ",0,9);

            switch (opcaoMenu){

                case 1:
                    h = instanciaHash(atributos);

                    comandoTmp = "IF NOT EXIST .\\"+FILEPATH_INS+" mkdir "+FILEPATH_INS;
                    system(comandoTmp.c_str());

                    if (!arquivos::imprimeArquivosINS()) break;

                    fileNameTmp = input::pegaRespostaStr(
                        "\nEntre o nome do arquivo que deseja usar (ou aperte apenas ENTER para cancelar):\n");
                    if (fileNameTmp == "")
                        break;
                    else
                        fileNameTmp += ".ins";

                    cout<<"\nIncluir buscas? (1-Sim; 2-Nao)\n";
                    opcaoTmp = input::pegaRespostaMinMax("",1,2);

                    try{
                        h = instanciaHash(atributos);
                        resultadoBMK = h->inserirDeArquivo(fileNameTmp, false);

                        if(opcaoTmp == 1);
                        tempoTotal_buscas = h->benchmarkBUSCA(0,fileNameTmp);

                        cout<<"\nTudo feito!\n\n";
                        cout<<"Tempo insercoes: "<<resultadoBMK.tempoBMK<<"ms\n";

                        if (opcaoTmp == 10)
                        cout<<"Tempo buscas:    "<<tempoTotal_buscas<<"ms\n";

                        cout<<"Colisoes:        "<<resultadoBMK.colisoes<<"\n";
                        cout<<"Rehashings:      "<<resultadoBMK.rehashings<<"\n";
                    }catch(bad_alloc e1){
                        cerr<<BAD_ALLOC_MSG;
                    }catch(arquivo_defeituoso e2){
                        cerr<<e2.what();
                    }

                    system("pause>0");
                    delete h;
                    break;

                case 2:
                case 3:
                case 4:
                case 5:
                    cout<<"\nQuantos?\n";
                    cin>>numeroTmp;

                    fflush(stdin);
                    cout<<"\nNome do arquivo de insercao a ser gerado (aperte apenas ENTER para usar o nome padrao):\n";
                    if (cin.peek() == '\n') {                   //se o proximo caractere eh um newline
                        fileNameTmp = DEFAULT_FILENAME_INS;   //atribui o nome padrao
                    } else{
                        cin>>fileNameTmp;                     //senao, atribui o nome digitado
                        fileNameTmp += ".ins";
                    }

                    cout<<"\nIncluir buscas? (1-Sim; 2-Nao)\n";
                    opcaoTmp = input::pegaRespostaMinMax("",1,2);

                    if (opcaoTmp == 1){
                        opcaoTmp = 10;
                    }

                    arquivos::inicializaDiretorioINS(fileNameTmp);

                    try{
                        h = instanciaHash(atributos);
                        resultadoBMK = h->benchmarkINSERCAO(numeroTmp,opcaoTmp,fileNameTmp);
                        if(opcaoTmp == 10);
                        tempoTotal_buscas = h->benchmarkBUSCA(0,fileNameTmp);

                        cout<<"\nTudo feito!\n\n";
                        cout<<"Tempo insercoes: "<<resultadoBMK.tempoBMK<<"ms\n";

                        if (opcaoTmp == 10)
                        cout<<"Tempo buscas:    "<<tempoTotal_buscas<<"ms\n";

                        cout<<"Colisoes:        "<<resultadoBMK.colisoes<<"\n";
                        cout<<"Rehashings:      "<<resultadoBMK.rehashings<<"\n";

                    }catch(bad_alloc &e){
                        cerr<<BAD_ALLOC_MSG;
                    }

                    system("pause>0");
                    delete h;
                    break;

                case 6:
                case 7:
                case 8:
                    cout<<"\nIncluir buscas? (1-Sim; 2-Nao)\n";
                    opcaoTmp = input::pegaRespostaMinMax("",1,2);

                    codigoBmkTmp = opcaoMenu;
                    if (opcaoTmp == 1){
                        codigoBmkTmp *= 10;      //Faz isso para poder identificar quando o benchmark inclui buscas
                                                        //(a variavel referente a opcao escolhida fica divisivel por 10)
                    }

                    tempoTotal_insercoes = 0;   //O somatorio dos tempos de cada insercao do benchmark
                    tempoTotal_buscas = 0;      //O somatorio dos tempos de cada busca do benchmark
                    tempoDecorrido = 0;         //O tempo real do processo inteiro de benchmark
                    rehashingsTotais = 0;
                    MAXcolisoes = 0;
                    qtdInsercoes = 1;
                    //Inicializa o diretorio do arquivo de benchmark de insercoes
                    arquivos::inicializaDiretorioBMK(arquivos::montarNomeDoArquivoBMK(atributos.tipo,codigoBmkTmp,true));

                    //E inicializa o diretorio do de buscas, caso isso tenha sido pedido
                    if ((opcaoTmp%10) == 0) arquivos::inicializaDiretorioBMK(arquivos::montarNomeDoArquivoBMK(atributos.tipo,codigoBmkTmp,false));

                    cout<<"\nTrabalhando...    0%";
                    try{
                        benchmark::cronometro cron;
                        while (true){
                            if ((opcaoTmp%10) == 0) arquivos::inicializaDiretorioINS(FILENAME_DUMMY);

                            if (qtdInsercoes > QTDMAX_INS)
                                qtdInsercoes = QTDMAX_INS;

                            h = instanciaHash(atributos);
                            resultadoBMK = h->benchmarkINSERCAO(qtdInsercoes, codigoBmkTmp, FILENAME_DUMMY);
                            if ((codigoBmkTmp%10) == 0) tempoTotal_buscas += h->benchmarkBUSCA(codigoBmkTmp, FILENAME_DUMMY)/1000;
                            delete h;

                            printNewPerc(qtdInsercoes,QTDMAX_INS);

                            tempoTotal_insercoes += resultadoBMK.tempoBMK/1000;
                            rehashingsTotais += resultadoBMK.rehashings;
                            if (resultadoBMK.colisoes > MAXcolisoes)
                                MAXcolisoes = resultadoBMK.colisoes;

                            if (qtdInsercoes == QTDMAX_INS)
                                break;
                            else
                                qtdInsercoes += GAP_INS;
                        }
                        tempoDecorrido = cron.tempoDecorrido();

                        cout<<"\n\nBenchmark completo!\n\n";
                        /*o "tempo decorrido" indica o tempo real decorrido do inicio ao fim do processo de
                        benchmark, pois inclui o tempo de leitura e escrita dos arquivos, o que eh muito mais
                        evidente em benchmarks incluindo buscas.*/
                        cout<<"Tempo decorrido: "<<tempoDecorrido/1000<<"s\n";
                        cout<<"Tempo insercoes: "<<tempoTotal_insercoes<<"s\n";

                        if ((codigoBmkTmp%10==0))
                        cout<<"Tempo buscas:    "<<tempoTotal_buscas<<"s\n";

                        cout<<"MAX Colisoes:    "<<MAXcolisoes<<"\n";
                        cout<<"Rehashings:      "<<rehashingsTotais<<"\n";

                    }catch(bad_alloc& ba){
                        cerr << BAD_ALLOC_MSG;
                    }

                    system("pause>0");
                    break;

                case 9:
                    loopMenu = false;
                    break;

                case 0:
                    printConstantes();
                    break;

                default:

                    cout<<"\n\nOpcao invalida!";
                    system("pause>0");

            }

        }

    }

}
