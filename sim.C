#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <alloc.h>
#include "lista.h"

FILE *Arch;    //Puntero Archivo de Entrada

int quantum=0,       //tmmano del quantum
    scheduler,       //tipo de Scheduler 0=Round Robin, 1=FIFO
    t_total=0,       //Tiempo total del sistma
    t_entrar=0,      //Tiempo que pueden entrar procesos
    t_cpuutil=0,     //Tiempo del CPU que se uso
    Resolucion=1,    //Resolucion del Sistema, tiene que ser mayor o igual que uno
    ProcessId=0,     //Numero de Proceso unico del ID
    pro_matados=0,   //# de procesos que fueron matados porque se les acabo su tiempo;
    pro_terminados=0;//# de procesos que han sido ejecutados totalmente.

float sumready=0;    //Sumatoria de Readys


class SubPrograma {
  public:
    int cpu,      //tiempo de CPU
        io,       //tiempo promedio de espera para I/O
        rep;      //# de veces que se hace un proceso
};

class Programa {
  public:
    char nombre[60];    //Nombre del Programa
    int landa,          //frecuencia con que entran programas
        veces,          //veces que ha entrado el programa
        tiempo;         //tiempo total del programa

    lista *l_SubProgramas;       //apuntador a una lista de programas

    Programa()
    { l_SubProgramas = new lista();
      veces = 0;
    };

    ~Programa()
    { delete l_SubProgramas;
    };
};

//----------------------------Class SubProceso------------------------------------
class SubProceso {
  public:
    int total_cpu,
        con_cpu,
        total_io,
        con_io,
        total_rep,
        con_rep;
    SubProceso();
    SubProceso(SubPrograma*);
};

SubProceso::SubProceso()
{ total_cpu =0;
  con_cpu = 0;
  total_io = 0;
  con_io = 0;
  total_rep = 0;
  con_rep =0;
}

SubProceso::SubProceso(SubPrograma *spt)
{ total_cpu = spt->cpu;
  con_cpu = 0;
  total_io = spt->io;
  con_io = 0;
  total_rep = spt->rep;
  con_rep = 0;
}
//----------------------------Fin Class SubProceso---------------------------------

//-----------------------------Class Proceso---------------------------------------
class Proceso {
  public:
    char nombre[60];
    int  tiempo,         //el tiempo que puede correr el programa
         contiempo,      //cuanto tiempo lleva corriendo
         tiempocpu,      //tiempo que esta en la cola de ready
         tiempoready,    //tiempo que esta en la cola de ready
         tiempoIO,       //tiempo que esta en la cola de IO
         Id;             //identificador unico del proceso;

    lista *l_SubProcesos;
    Proceso();
    Proceso(Programa*);
    ~Proceso();
};

Proceso::Proceso()
{ tiempo = 0;
  contiempo = 0;
  tiempocpu = 0;
  tiempoready = 0;    //tiempo que esta en la cola de ready
  tiempoIO = 0;       //tiempo que esta en la cola de IO
  nombre[0]='\0';
  l_SubProcesos = new lista();
};


Proceso::Proceso(Programa *pt)     //recive un programa y lo vuelve proceso
{ tiempo = pt->tiempo;
  contiempo = 0;
  tiempocpu = 0;
  tiempoready = 0;    //tiempo que esta en la cola de ready
  tiempoIO = 0;       //tiempo que esta en la cola de IO
  strcpy(nombre,pt->nombre);
  l_SubProcesos = new lista();
  int i;
  SubProceso *spt=NULL;
  for (i=0;i<pt->l_SubProgramas->Count();i++) //ingresa los Subprocesos
  { spt = new SubProceso((SubPrograma*) (*pt->l_SubProgramas)[i]);
    //printf("Puntero a subproceso: %i\n",spt);
    //printf("cpu %i, IO %i, rep %i \n",spt->con_cpu, spt->con_io,spt->con_rep);
    l_SubProcesos->Insert(spt);
    //printf("Numero de subsprocesos %d \n", l_SubProcesos->Count());
  }
}
Proceso::~Proceso()
{ delete l_SubProcesos;
}
//----------------------------Fin Class Proceso-----------------------------------

lista *l_Programas,         //lista de progrmas
      *l_Ready,             //lista de procesos en ready
      *l_IO;                //lista de procesos en IO

//---------------------------- Abre Archivo -----------------------------
bool OpenFile(char *NomArch)
{ bool existe=true;
  if ( NomArch == NULL)
  { NomArch = new char;
    printf("Ingrese nombre del Archivo: ");
    scanf("%s",NomArch);
  }
  if ((Arch = fopen(NomArch, "rt")) == NULL)
    existe = false;
  else
    existe = true;
  return existe;
} //OpenFile
//--------------------- Fin Abre y crea Archivos ------------------------


int estado,  //Numero de Estado
    ccolum,  //Numero de Columna
    clinea,  //Numero de Linea
    tamtok,  //Tamano del Token
    ct,      //Contador lexema o variable *token
    ca,      //Contador del Archivo
    error;


char *token;


//------------------- Verificarciones de Conjuntos ----------------------
bool GenScanLetra(unsigned char car)
{
  if ( ((car>='A') && (car<='Z')) ||
       ((car>='a') && (car<='z')) ||
        (car=='_') )
    return true;
  else
    return false;
}

bool GenScanDigito(unsigned char car)
{
  if ( ((car>='0') && (car<='9')) )
    return true;
  else
    return false;
}

//----------------- Fin Verificarciones de Conjuntos --------------------

void Regreso()
{ ca--;   // regresa uno en el archivo
  ct--;
  token[ct]='\0';
  ccolum--;
}

int TomaToken()
{ estado = 0;              //Estado del TomaToken()
  unsigned char car = 0;   //Caracter que se toma del Archivo
  bool Aceptacion=false;   //cuando esta en un estado de aceptacion
  ct = 0;                  //contador del lexema o variable *token
  fseek(Arch, ca, SEEK_SET);
  while ( (!Aceptacion) && (!feof(Arch)) )
  { memset (token,0,tamtok);
    while ( (!Aceptacion) && (!feof(Arch)) )
    { car = fgetc(Arch);
      ca++;
      if (car!='\n') ccolum++;
      token[ct++]=car;
      switch (estado)
      { case 0:
          switch (car)
//----------------------- caracteres separadores-------------------------
          { case '\r':
              ccolum = 0;
              estado = 0;
              memset(token, 0, tamtok);
              ct = 0;
            break;
            case '\n':
              clinea++;
              if (ccolum!=0) ca++; //se suma uno ca porque no lee el caracter \r
              ccolum = 0;
              estado = 0;
              memset(token, 0 , tamtok);
              ct = 0;
            break;
            case '\t': case ' ':
              estado = 0;
              memset(token, 0, tamtok);
              ct = 0;
            break;
//----------------------- caracteres separadores-------------------------
            default:
              if (GenScanLetra(car))
                estado = 1;
              else if (GenScanDigito(car))
                estado = 2;
              else
              { estado = 99;
                Aceptacion = true;
              } //else
          } // switch (car)
        break; //case 0
        case 1:
          switch (car) {
            default:
              if (GenScanLetra(car))
                estado = 3;
              else if (GenScanDigito(car))
                estado = 3;
              else
              { Aceptacion = true;
                Regreso();
              }
          } //switch (car)
        break; //case 1
        case 2:
          switch (car) {
            default:
              if (GenScanDigito(car))
                estado = 2;
              else
              { Aceptacion = true;
                Regreso();
              }
          } //switch (car)
        break; //case 2
        case 3:
          switch (car) {
            default:
              if (GenScanLetra(car))
                estado = 3;
              else if (GenScanDigito(car))
                estado = 3;
              else
              { Aceptacion = true;
                Regreso();
              }
          } //switch (car)
        break; //case 3
      } //switch estado
    } //while ( (!Aceptacion) && (car = fgetc(Arch) != EOF) )
  } //while ( (!Aceptacion) && (ArchTake ==0) )
//----------------------- Codigo de Actions -------------------------
  switch (estado)
  {
    case 1:
      if (strcmp("Q", token) == 0) estado = 10;        //quantum
      else if (strcmp("S", token) == 0) estado = 11;   //tipo de Scheduller
      else if (strcmp("F", token) == 0) estado = 12;   //First come First Served
      else if (strcmp("R", token) == 0) estado = 13;   //Round Running
      else if (strcmp("T", token) == 0) estado = 14;   //Timepo de la maquina en el que entran procesos
      else if (strcmp("P", token) == 0) estado = 20;   //Nombre del Proceso
      else if (strcmp("l", token) == 0) estado = 21;   //Frecuencia de entrada
      else if (strcmp("t", token) == 0) estado = 22;   //Tiempo total de corrida
      else if (strcmp("c", token) == 0) estado = 30;   //CPU time
      else if (strcmp("i", token) == 0) estado = 31;   //I/O time
      else if (strcmp("r", token) == 0) estado = 32;   //Numero de veces
    break;
  } //switch
//--------------------- Fin Codigo de Actions -----------------------
  return estado;
} //TomaToken

void GramRegreso()
{ int l;
  l = strlen (token) + 1;
  ca = ca - l;
  ccolum = ccolum - l;
}

void Valores()
{ int t=0;
  if (!error)
  { while ( t != 20)
    { t = TomaToken();
      switch (t)
      { case 10:
          if (TomaToken() == 2)
            quantum = atoi(token);
          else
            error = 3; //se espera un numero
        break;
        case 11:
          t = TomaToken();
          if  ( t == 13) //una R
            scheduler = 0;
          else  if ( t == 12) //una F
            scheduler = 1;
          else
            error = 1; //se espera una F o una R
        break;
        case 14:
          if (TomaToken() == 2)
            t_entrar = atoi(token);
          else
            error = 3; //se espera un numero
          t_entrar = atoi(token);
        break;
        case 20:break;
        default:
          error = 2; //se espera
      }// switch
    } //while
    GramRegreso();
  }
}

void GuardaProgramas()
{ while ( (TomaToken() == 20) && (!error) )
  { Programa *tp = new Programa();
    if (TomaToken() == 3)
      strcpy(tp->nombre, token);
    else
      error = 4; //se espera el nombre del proceso
    if (TomaToken() == 21)       // "l" landa
    { if (TomaToken() == 2)      // numero
        tp->landa = atoi(token);
      else
        error = 3; //se espera un numero
    }
    else
      error = 5; //se espera landa
    if (TomaToken() == 22)       // "t" tiempo total de corrida
    { if (TomaToken() == 2)      // numero
        tp->tiempo = atoi(token);
      else
        error = 3; //se espera un numero
    }
    else
      error = 6; //se espera t
    while ( (TomaToken() == 30) && (!error) )
    { SubPrograma *tps = new SubPrograma();
      if (TomaToken() == 2)      // numero
        tps->cpu = atoi(token);
      else
        error = 3; //se espera un numero

      if (TomaToken() == 31)       // "i" io time
        if (TomaToken() == 2)      // numero
          tps->io = atoi(token);
        else
          error = 3; //se espera un numero
      else
        error = 7; //se espera IO

      if (TomaToken() == 32)       // "r" io time
        if (TomaToken() == 2)      // numero
          tps->rep = atoi(token);
        else
          error = 3; //se espera un numero
      else
        error = 8; //se espera r
      if (!error)
      { //printf("Puntero %d\n", tps);
        tp->l_SubProgramas->Insert(tps); //inserte el subprograma
      }
    } //while
    GramRegreso();
    //printf("error: %d\n",error);
    if (!error)
    { l_Programas->Insert(tp);
      printf("Programa %s guardado en estructura con %i subprograma(s)\n"
             ,tp->nombre,tp->l_SubProgramas->Count());
    }
  } //while
  printf("\nExisten %d tipos de programa(s)\n\n", l_Programas->Count() );
  GramRegreso();
}
void Gramatica()
{ Valores();
  GuardaProgramas();
}

void lee_archivo(char *NomArch)
{ if (OpenFile(NomArch))
  { ca = 0;
    estado = 0;
    ccolum = 0;
    clinea = 1;
    error = 0;
    tamtok = 60;   //Tamano maximo del token
    token = (char *) malloc(tamtok);
    int EstTom=0;  //estado que toma el token
    fseek(Arch, 0, SEEK_SET);
    Gramatica();
    //while (!feof(Arch))
    //{ EstTom = TomaToken();
    //  printf("%d ", strlen(token));
    //  printf("Linea: %i, Columna: %i -> Token: %s = %i\n",
    //          clinea ,ccolum, token, EstTom);
    //} //while
    fclose(Arch);
  }
  else
    printf("No existe el archivo\n");
} //Scanner


void SumaReadys(int cuanto)
{ int i = 0;
  Proceso *pt;
  for (i=0;i<l_Ready->Count();i++)
  { pt = (Proceso*) (*l_Ready)[i];
    pt->tiempoready = pt->tiempoready + cuanto;
    sumready = sumready + cuanto;
  }
} //SumaReadys

void SumaTiemposIO(int cuanto)
{ int i;
  Proceso *pt;
  SubProceso *spt;
  //printf("Numero de Procesos en IO: %i\n",l_IO->Count());
  for (i=0;i<l_IO->Count();i++)
  { pt = (Proceso*) (*l_IO)[i];
    pt->tiempoIO = pt->tiempoIO + cuanto;
    pt->contiempo = pt->contiempo + cuanto;
    spt = (SubProceso*) pt->l_SubProcesos->UsarActual();
    if (spt->con_io >= spt->total_io) //si ya termino el tiempo de io
    { spt->con_rep++;
      spt->con_io=0;
      if (spt->con_rep >= spt->total_rep) //si se acaban las repeticiones
      { pt->l_SubProcesos->DerechaActual();
        if (pt->l_SubProcesos->UsarActual() == NULL) //si se acabo el proceso
        { //printf("%d %s %d %d",t_total, pt->nombre, pt->Id, pt->tiempoready);
         printf("  %d: Se completo el proceso %s, con id %d\n"
                "  Su tiempo en la Cola de Ready fue: %d\n"
                , t_total,pt->nombre,pt->Id,pt->tiempoready);
          pro_terminados++;
          l_IO->Kill(i);    //lo mata de IO, desaparece el proceso de memoria
        }
        else //si no se acabo el proceso
        { l_Ready->Insert(pt);  //mete el proceso en la cola de ready;
          l_IO->Eliminar(i);    //lo quita de IO
          printf("%d: Salio de IO el proceso %s, con id %d a la cola de Ready\n",
                  t_total,pt->nombre,pt->Id);
        }
      }
      else //si no se acaban las repeticones
      { l_Ready->Insert(pt);  //mete el proceso en la cola de ready;
        l_IO->Eliminar(i);    //lo quita de IO
        printf("%d: Salio de IO el proceso %s, con id %d a la cola de Ready\n",
                t_total,pt->nombre,pt->Id);
      }
    }
    spt->con_io = spt->con_io + cuanto;
    //printf("c %d, i %d, r %d\n",spt->con_cpu, spt->con_io, spt->con_rep);
    //printf("t %d, t %d, t %d\n",spt->total_cpu, spt->total_io, spt->total_rep);
  } //for
}

void IngresoProcesos()
{ int i = 0,
      j = 0,
      c = 0;
  Proceso *pt = NULL;
  Programa *pr = NULL;
  for (i=0;i<l_Programas->Count();i++)
  { pr = (Programa*) (*l_Programas)[i];
    c = (t_total / pr->landa) - pr->veces;
    for (j=0;j<c;j++)
    { pt = new Proceso(pr);
      pt->Id = ProcessId;
      ProcessId++;
      l_Ready->Insert(pt); //inserta el proceso
      pr->veces++;
      printf("  %d: Entro el programa %s, como proceso con el ID %i\n"
             "  con %i subprogramas en la cola de ready\n"
             ,t_total,pt->nombre,pt->Id,(pt->l_SubProcesos)->Count());
    }
    //pr = NULL;
  } //for
} //IngresoProcesos()

void RoundRobin()
{ Proceso *PCPU; //proceso en CPU
  SubProceso *SPCPU;
  int acu,i;
  //bool salir = false;
  //if ( (t_total > t_entrar) && (l_Ready->Count()==0) && (l_IO->Count()==0)) salir = true;
  //Mientras el tiempo del sistema es menor que el tiempo en el que pueden entrar procesos
  while ((t_total <= t_entrar) || (l_Ready->Count()>0) || (l_IO->Count()>0) )
  { //if (l_Ready->Count()>0)
    //{ PCPU = (Proceso *) (*l_Ready)[0];
    //}
    if (l_Ready->Count()>0)  // Mete el Proceso a al CPU si hay en Ready
    { PCPU = (Proceso* ) l_Ready->SacarFirst();
      printf("%d: Entro al CPU el proceso %s, con Id %d\n", t_total, PCPU->nombre, PCPU->Id);
      SPCPU = (SubProceso* ) PCPU->l_SubProcesos->UsarActual();
      acu = 0;
      //while donde el cpu no puede hacer nada mas que correr el programa
      //Mientras no se le acaba su tiempo al proceso o no llega a pedir IO
      while ( (PCPU->contiempo <= PCPU->tiempo) &&
              (SPCPU->con_cpu < SPCPU->total_cpu) &&
              (acu <= quantum) )
      { //printf("%d: \n", t_total+acu );
        acu = acu + Resolucion;
        SPCPU->con_cpu = SPCPU->con_cpu + Resolucion;
        PCPU->contiempo = PCPU->contiempo + Resolucion;
      }
      //printf("Subproceso contador de tiempo: %d \n", SPCPU->con_cpu);
      t_cpuutil = t_cpuutil + acu; //suma el tiempo util a cpu.
      t_total = t_total + acu;     //suma el tiempo al total del sistema
      SumaReadys(acu);
      if (PCPU->contiempo > PCPU->tiempo) //si al proceso se le acabo su tiempo.
      { printf("%d: se acabo el tiempo del proceso %s con Id: %d\n"
               "  Su tiempo en la Cola de Ready fue: %d\n"
               ,t_total, PCPU->nombre, PCPU->Id, PCPU->tiempoready);
        delete PCPU; //Mata al proceso porque excedio de su tiempo previsto
        pro_matados++;
      }
      else if (SPCPU->con_cpu >= SPCPU->total_cpu) //si pide un IO
      { SPCPU->con_cpu = 0;
        l_IO->Insert(PCPU);  // inserta el proceso en la lista de IO
        printf("%d: Salio del CPU el proceso %s con Id: %d a la cola de IO\n"
               ,t_total, PCPU->nombre, PCPU->Id);
      }
      else if (acu > quantum)
      { l_Ready->Insert(PCPU);
      } //else if
      SumaTiemposIO(acu);   //suma tiempos de IO;
      //printf("Acumulado: %i PCPU->contiempo: %d\n",acu, PCPU->contiempo);
      //printf("Io procesos %d\n",l_IO->Count());
      //printf("ready procesos %d\n",l_Ready->Count());
      //printf("Proceso: %s, con tiempo: %d\n", PCPU->nombre, PCPU->contiempo);
      //printf("Cpu:%d, IO:%d rep:%d\n", SPCPU->total_cpu, SPCPU->con_io, SPCPU->con_rep );
    } //if
    else //No hay procesos en la cola de ready;
    { t_total = t_total + Resolucion;
      SumaTiemposIO(Resolucion);
      //printf("else Io procesos %d\n",l_IO->Count());
      //printf("else acumulador %d\n",acu);
      //printf("else: Io procesos %d\n",l_IO->Count());
      //printf("else: ready procesos %d\n",l_Ready->Count());
    } //else
//    for (i=t_total;i<=t_total + acu;i++)
    if (t_total <= t_entrar) IngresoProcesos();  //revisar landas

    //printf("Tiempo del sistema: %d\n", t_total);
  } //while
}

void FCFS()
{ Proceso *PCPU; //proceso en CPU
  SubProceso *SPCPU;
  int acu,i;
  while ((t_total <= t_entrar) || (l_Ready->Count()>0) || (l_IO->Count()>0) )
  { if (l_Ready->Count()>0)  // Mete el Proceso a al CPU si hay en Ready
    { PCPU = (Proceso* ) l_Ready->SacarFirst();
      printf("%d: Entro al CPU el proceso %s, con Id %d\n", t_total, PCPU->nombre, PCPU->Id);
      SPCPU = (SubProceso* ) PCPU->l_SubProcesos->UsarActual();
      acu = 0;
      //while donde el cpu no puede hacer nada mas que correr el programa
      //Mientras no se le acaba su tiempo al proceso o no llega a pedir IO
      while ( (PCPU->contiempo <= PCPU->tiempo) &&  (SPCPU->con_cpu < SPCPU->total_cpu) )
      { acu = acu + Resolucion;
        SPCPU->con_cpu = SPCPU->con_cpu + Resolucion;
        PCPU->contiempo = PCPU->contiempo + Resolucion;
      }
      t_cpuutil = t_cpuutil + acu; //suma el tiempo util a cpu.
      t_total = t_total + acu;     //suma el tiempo al total del sistema
      SumaReadys(acu);
      if (PCPU->contiempo > PCPU->tiempo) //si al proceso se le acabo su tiempo.
      { printf("%d: se acabo el tiempo del proceso %s con Id: %d\n"
               "  Su tiempo en la Cola de Ready fue: %d\n"
               ,t_total, PCPU->nombre, PCPU->Id, PCPU->tiempoready);
        delete PCPU; //Mata al proceso porque excedio de su tiempo previsto
        pro_matados++;
      }
      else if (SPCPU->con_cpu >= SPCPU->total_cpu) //si pide un IO
      { SPCPU->con_cpu = 0;
        l_IO->Insert(PCPU);  // inserta el proceso en la lista de IO
        printf("%d: Salio del CPU el proceso %s con Id: %d a la cola de IO\n"
               ,t_total, PCPU->nombre, PCPU->Id);
      }
      SumaTiemposIO(acu);   //suma tiempos de IO;
    } //if
    else //No hay procesos en la cola de ready;
    { t_total = t_total + Resolucion;
      SumaTiemposIO(Resolucion);
    } //else
//    for (i=t_total;i<=t_total + acu;i++)
    if (t_total <= t_entrar) IngresoProcesos();  //revisar landas
  } //while
}

void Boot()
{ int i;
  Proceso *pt = NULL;
  //ingresa todos los procesos por primera vez a la lista de ready
  printf("      Scheduler: ");
  if (scheduler == 0) printf("Round Robin\n\n");
  else printf("First Come First Served\n\n");
  for (i=0;i<l_Programas->Count();i++)
  { pt = new Proceso( (Programa*) (*l_Programas)[i] );
    pt->Id = ProcessId;
    ProcessId++;
    l_Ready->Insert(pt); //inserta el proceso
    printf("Entro el programa %s, como proceso con el ID %i\n"
           "con %i subprogramas en la cola de ready\n"
           ,pt->nombre,pt->Id,(pt->l_SubProcesos)->Count());
  }
  printf("\n");
  if (scheduler == 0) RoundRobin();
  else if (scheduler == 1) FCFS();

} //Scheduller;

void main(int argc, char *argv[])
{ l_Programas = new lista();
  l_Ready = new lista();
  l_IO = new lista();
  printf("\n\nProyecto Simulador de Procesos\n\n");
  lee_archivo(argv[1]);
  Boot();
  float porutil = (float) t_cpuutil * 100 / (float) t_total,
        awt = sumready / (pro_terminados+pro_matados),
        at = (float) (pro_terminados+pro_matados) / t_total;
  printf("\n\nEstadisticas\n\n");
  printf("Tamano del quantum (si es RR aplicable): %d\n",quantum);
  printf("Scheduler                              : ");
  if (scheduler == 0) printf("Round Robin\n");
  else printf("First Come First Served\n");
  printf("Tiempo que pueden entrar procesos      : %d\n",t_entrar);
  printf("# de procesos ejecutados totalmente    : %d\n",pro_terminados);
  printf("# de procesos que se le acabo su tiempo: %d\n",pro_matados);
  printf("Tiempo util de CPU                     : %d\n",t_cpuutil);
  printf("Tiempo total del sistema               : %d\n",t_total);
  printf("Troughput                              : %f\n", at);
  printf("Average Waiting Time                   : %f\n", awt);
  printf("Porcentaje de ocupacion del CPU        : %f%\n",porutil);
//  printf("Cola de IO                             : %d\n",l_IO->Count());
//  printf("Cola de Ready                          : %d\n",l_Ready->Count());
//  printf("Error                                  : %d\n\n",error);
} //main
