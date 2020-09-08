//clase lista
#ifndef  __lista_h__

class nodo {
  friend class lista;
    void *puntero;
    nodo *prev,
         *next;

  public:
    nodo* GetNext();
    nodo* GetPrev();
    nodo();
};

class lista {
  protected:
    nodo *Head,
         *Actual;
    bool Empty();
    int nc;

  public:
    lista();
    void Insert(void *);   //inserta un elemento a la lista
    int Count();
    nodo* GetFirst();
    void* operator[](int);
    void Eliminar(int); //elimina el puntero de la lista, pero no lo quita de memoria
    void Kill(int);     //elimina el puntero de la lista, y de la memoria
    void* SacarFirst();
    void* UsarActual();
    void RoundDerechaActual();
    void DerechaActual();
    ~lista();
};


#endif