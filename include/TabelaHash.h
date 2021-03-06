/*
 * File:   TabelaHash.h
 * Author: Daniel Franklin
 *
 * Created on 2018
 * Part of the DaniHash project
 */

#ifndef TABELAHASH_H_INCLUDED
#define TABELAHASH_H_INCLUDED

#include "EstruturaAuxiliar.h"

#include "utilBenchmark.h"
#include "utilArquivos.h"

namespace dh{

    class TabelaHash{

        public:

        int TH, tipo;

        virtual void inserir(int chave, bool PI)=0;
        virtual void remover(int chave, bool PI)=0;
        virtual int buscar(int chave, bool PI)=0;
        virtual void imprimir()=0;
        virtual float getFC()=0;
        virtual int getColisoesDaInsercaoAtual()=0;
        virtual bool getFezRehashing()=0;

        int getTipo(){
            return tipo;
        }

        bmk::Results realizarInstrucoesDeArquivo(string nomeDoArquivo, bool PI){

            int numeroNaLinha, numeroDaLinha = 0, tamanhoDaLinha, indiceInicial;
            string linha, operacao, opAnterior = "";
            ifstream fileREAD;
            fileREAD.open(cons::FILEPATH_INS+nomeDoArquivo);
            bmk::Results resultado;
            bmk::cronometro cron;

            if (!fileREAD)
                throw exc::excecao_arquivo(exc::EX_FILE_FILENOTFOUND,cons::FILEPATH_INS+nomeDoArquivo);

            while(fileREAD){
                numeroDaLinha++;
                getline(fileREAD, linha);
                tamanhoDaLinha = linha.size();
                indiceInicial = arq::getIndiceInicial(linha);

                if (linha.empty()){
                    continue;
                }else if (linha.at(indiceInicial) == '#'){
                    if (linha.at(indiceInicial)  == '#' &&
                        linha.at(indiceInicial+1)!= '#'){

                        out::printNoPause(PI," * ",linha.substr(
                        indiceInicial+1,tamanhoDaLinha-indiceInicial+1)," * \n");
                    }
                }else{
                    if (tamanhoDaLinha >= 5){
                        operacao = linha.substr(indiceInicial,4);

                        try{
                            numeroNaLinha = stoi(linha.substr(indiceInicial+4,tamanhoDaLinha-4));
                        }catch(invalid_argument e){
                            throw exc::excecao_arquivo(exc::EX_FILE_INVALIDCHAR,to_string(numeroDaLinha));
                        }
                    }else{
                        throw exc::excecao_arquivo(exc::EX_FILE_INCOMPLETELINE,to_string(numeroDaLinha));
                    }

                    if(operacao =="INS "){
                        if (opAnterior != "INSERCAO"){
                            opAnterior = "INSERCAO";
                            out::printNoPause(PI,"INSERCAO:\n");
                        }
                        cron.reset();
                        this->inserir(numeroNaLinha, PI);
                        resultado.tempoBMK += cron.tempoDecorrido();

                        resultado.colisoes += getColisoesDaInsercaoAtual();

                        if (this->getFezRehashing())
                            resultado.rehashings++;

                    }else if (operacao == "DEL "){
                        if (opAnterior != "DELECAO"){
                            opAnterior = "DELECAO";
                            out::printNoPause(PI,"DELECAO:\n");
                        }
                        cron.reset();
                        this->remover(numeroNaLinha, PI);
                        resultado.tempoBMK += cron.tempoDecorrido();

                        resultado.colisoes += getColisoesDaInsercaoAtual();

                    }else if (operacao == "BSC "){
                        if (opAnterior != "BUSCA"){
                            opAnterior = "BUSCA";
                            out::printNoPause(PI,"BUSCA:\n");
                        }
                        cron.reset();
                        out::printResultadoBusca(PI,this->buscar(numeroNaLinha, PI));
                        resultado.tempoBMK += cron.tempoDecorrido();

                        resultado.colisoes += getColisoesDaInsercaoAtual();

                    }else{
                        fileREAD.close();
                        throw exc::excecao_arquivo(exc::EX_FILE_NOIDENTIFIER,to_string(numeroDaLinha));
                    }
                }
            }
            fileREAD.close();
            return resultado;
        }

        bmk::Results benchmarkINSERCAO(int quantidadeDeInsercoes, int opcao_insbmk, string fileName_insercao){
            int chaveParaInserir;
            double tempo = 0;
            ofstream fileINS;
            bmk::Results resultado;

            if (bmk::benchmarkComArquivoDeInsercao(opcao_insbmk)){
                fileINS.open(cons::FILEPATH_INS+fileName_insercao, ios::app);
            }

            for (int i = 0; i < quantidadeDeInsercoes; i++){
                //Geramos a variavel aleatoria a ser inserida
                chaveParaInserir = rand::getRandom(opcao_insbmk);

                if (chaveParaInserir < 0) chaveParaInserir = 0;

                if (bmk::benchmarkComArquivoDeInsercao(opcao_insbmk))
                    fileINS<<"INS "<<chaveParaInserir<<endl;

                bmk::cronometro cron;
                inserir(chaveParaInserir, false);   //Depois fazemos a insercao, medindo o tempo
                tempo += cron.tempoDecorrido();

                resultado.colisoes += getColisoesDaInsercaoAtual();
                if (this->getFezRehashing())
                    resultado.rehashings++;
            }
            resultado.tempoBMK = tempo;

            if (bmk::benchmarkComArquivoDeInsercao(opcao_insbmk))
                fileINS.close();

            //Se for um benchmark completo, inclui uma adicao ao arquivo com os tempos do bmk atual
            if (bmk::benchmarkComArquivoBenchmark(opcao_insbmk)){
                ofstream fileBMK;

                fileBMK.open(cons::FILEPATH_BMK+arq::montarNomeDoArquivoBMK(tipo,opcao_insbmk,true),ios::app);
                //Poe no arquivo o tempo da insercao
                fileBMK<<quantidadeDeInsercoes<<"\t"<<tempo<<endl;
                fileBMK.close();
            }

            return resultado;
        }

        double benchmarkBUSCA(int opcao_insbmk, string fileName_insercao){
            //Para cada numero inserido pelo arquivo de instrucoes, realizamos uma busca
            int numeroNaLinha, quantidadeDeBuscas = 0;
            double tempo = 0;
            string linha, ins_str;

            ifstream fileREAD;
            fileREAD.open(cons::FILEPATH_INS+fileName_insercao);  //Abre o arquivo de instrucoes gerado antes, pra saber o que deve ser buscado

            while(fileREAD){
                getline(fileREAD, linha);
                ins_str = linha.substr(0,3);
                if(ins_str=="INS"){
                    quantidadeDeBuscas++;
                    numeroNaLinha = stoi(linha.substr(4,linha.size()-4));

                    bmk::cronometro cron;
                    buscar(numeroNaLinha,false);
                    tempo += cron.tempoDecorrido();

                }else if (linha.empty() || linha.front() == '#' ||
                          ins_str=="DEL" || ins_str=="BSC"){
                    continue;
                }else{
                    cout<<"Arquivo com problema!"<<endl;
                    break;
                }
            }
            fileREAD.close();

            if (bmk::benchmarkComArquivoBenchmark(opcao_insbmk)){
                ofstream fileBMK;

                fileBMK.open(cons::FILEPATH_BMK + arq::montarNomeDoArquivoBMK(tipo,opcao_insbmk,false),ios::app);

                fileBMK<<quantidadeDeBuscas<<"\t"<<tempo<<endl;
                fileBMK.close();
            }

            return tempo;

        }

        virtual ~TabelaHash(){};

    };

    struct Atributos{

        public:

        int tipo, tamanho, limite;
    };

    Atributos pegaAtributosDaHash(bool trueBASIC_falseBMK){
        int opcao1, opcao2, tipoEscolhido = 0;
        Atributos atr;

        cout<<"Que tipo de hash deseja utilizar?\n\n";
        cout<<"1) Open Hashing com Lista\n";
        cout<<"2) Open Hashing com Arvore\n";
        cout<<"3) Closed Hashing\n";
        cout<<"4) Half-Open Hashing\n";
        cout<<"5) Sair\n\n";

        opcao1 = in::pegaRespostaMinMax("","Opcao: ",1,5);

        if (opcao1 == 1 || opcao1 == 2){
            tipoEscolhido = opcao1;
        }if (opcao1 == 3){

            system("cls");
            cout<<"Escolha como lidar com colisoes:\n\n";
            cout<<"1) Tentativa linear\n";
            cout<<"2) Tentativa quadratica\n";
            cout<<"3) Duplo hashing\n\n";

            opcao2 = in::pegaRespostaMinMax("","Opcao: ",1,3);

                 if (opcao2 == 1) tipoEscolhido = 3;
            else if (opcao2 == 2) tipoEscolhido = 4;
            else if (opcao2 == 3) tipoEscolhido = 5;

        }else if (opcao1 == 4){

            system("cls");
            cout<<"Deseja que a hash utilize tentativa quadratica?\n\n";
            cout<<"1) Sim, insercoes em arvores cheias contam como colisoes (CQT)\n";
            cout<<"2) Nao, pode continuar inserindo em arvores cheias (SQT)\n\n";

            opcao2 = in::pegaRespostaMinMax("","Opcao: ",1,2);

                 if (opcao2 == 1) tipoEscolhido = 6;
            else if (opcao2 == 2) tipoEscolhido = 7;

        }else if (opcao1 == 5){
            //Para forcar a ir logo pro final
            tipoEscolhido = 0;
            trueBASIC_falseBMK = false;
        }

        atr.tipo = tipoEscolhido;
        atr.limite = 1; //so pra deixar inicializado com algum valor

        if (trueBASIC_falseBMK == true){
            atr.tamanho = in::pegaRespostaMin("\nTamanho da tabela: ", 1);

            if (tipoEscolhido >= 6)
                atr.limite = in::pegaRespostaMin("Altura maxima das arvores: ", 1);

        }else{
            atr.tamanho = cons::TAM_INICIAL_HASH_BMK;
            atr.limite = cons::ALT_MAX_BMK;
        }

        return atr;

    }
};

#include "OpenHashing.h"
#include "ClosedHashing.h"
#include "HalfOpenHashing.h"

namespace dh{

    TabelaHash* instanciaHash(Atributos attr){

        if (attr.tipo <= 2)
            return new Ohash(attr.tamanho,attr.tipo);
        else if (attr.tipo <= 5)
            return new Chash(attr.tamanho,attr.tipo);
        else
            return new HOhash(attr.tamanho,attr.tipo,attr.limite);

    }
};

//
#endif // TABELAHASH_H_INCLUDED
