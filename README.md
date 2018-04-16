# cthread

O objetivo deste trabalho é a aplicação dos conceitos de sistemas operacionais relacionados ao escalonamento e ao contexto de execução, o que inclui a criação, chaveamento e destruição de contextos. Esses conceitos foram empregados no desenvolvimento de uma biblioteca de threads em nível de usuário (modelo N:1). Essa biblioteca de threads, denominada de compact thread (ou apenas cthread), oferece capacidades básicas para programação com threads como criação, execução, sincronização, término e trocas de contexto.

Para documentação detalhada, consulte o arquivo Cthreads 2018-01-C.pdf

![alt text](https://github.com/LucasAlegre/cthread/blob/master/cthread.png)

### Installing

Para gerar a biblioteca libcthread.a no diretório lib, execute o comando:

```
make
```

Para limpar:

```
make clean
```

## Rodando os testes

```
cd testes
make
./teste
```

### Estrutura de Diretórios e Arquivos

.
└── cthread
    ├── bin
    │   └── support.o
    ├── Cthreads 2018-01-C.pdf
    ├── exemplos
    │   ├── barbeiro.c
    │   ├── exemplo.c
    │   ├── filosofos.c
    │   ├── Makefile
    │   ├── mandel.c
    │   ├── prodcons.c
    │   ├── series.c
    │   ├── teste_vetor.c
    │   └── testSupport.c
    ├── include
    │   ├── cdata.h
    │   ├── cthread.h
    │   └── support.h
    ├── lib
    ├── Makefile
    ├── src
    │   └── cthread.c
    ├── support.pdf
    └── testes

## Authors

* **Lucas Alegre** - [LucasAlegre](https://github.com/LucasAlegre)
* **Aline Weber** - [alineweber](https://github.com/alineweber)
* **Lucas Hagen** - [LucasHagen](https://github.com/LucasHagen)
