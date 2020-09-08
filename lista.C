#include <stdio.h>
#include "lista.h"

nodo::nodo()
{ puntero = NULL;
}
nodo* nodo::GetNext()
{ return next;
}

nodo* nodo::GetPrev()
{ return prev;
}

lista::lista()
{ Head = NULL;
  Actual = NULL;
  nc=0;
} //constructor

bool lista::Empty()
{ if (Head == NULL) {
    return true;
  }
  else
    return false;
} //empty

void lista::Insert(void *datos)
{ nodo *p,*q;
  nc++;
  if (Empty())
  { p = new nodo;
    Head = p;
    Actual = Head;
    p->puntero = datos;
    //printf("Datos   %d\n", datos);
    //printf("punter  %d\n", p->puntero);
    p->prev = NULL;
    p->next = NULL;
  }
  else {
    p = Head;
    while (p->next != NULL) p=p->next;
    q = new nodo;
    q -> puntero = datos;
    //printf("Datos   %d\n", datos);
    //printf("punter  %d\n", q->puntero);
    q -> next = NULL;
    q -> prev = p;
    p -> next = q;
  } //else
} //insert

int lista::Count()
{ return nc;
}

nodo* lista::GetFirst()
{ return Head;
}

void* lista::operator[](int ind)
{ nodo *p;
  p = NULL;
  //printf("Ya Pase operado []\n");
  if ( (ind >= 0) && (ind < nc) )
  { int i=0;
    p = Head;
    while (i < ind)
    { p = p->next;
      i++;
    }
  }
  return p->puntero;
}

void lista::Eliminar(int ind)
{ //printf("Ya Pase Eliminar\n");
  nodo *p,*q;
  if ( (ind >= 0) && (ind < nc) )
  { int i=0;
    p = Head;
    while (i < ind)
    { p = p->next;
      i++;
    }
    if (p==Head)  //si es la cabeza
    { if (p->next == NULL)
      { Head = NULL;
        Actual = NULL;
      }
      else
      { if (Actual == Head)
          Actual = Actual->next;
        Head = Head->next;
        Head->prev = NULL;
      }
    }
    else
    { q = p->prev;
      if (Actual == p) Actual = q;
      q->next = p->next;
      if (p->next != NULL)
        p->next->prev = q;
    } //else
    nc--;
  } //if
} //Eliminar

void lista::Kill(int ind)
{ //printf("Ya Pase Eliminar\n");
  nodo *p,*q;
  if ( (ind >= 0) && (ind < nc) )
  { int i=0;
    p = Head;
    while (i < ind)
    { p = p->next;
      i++;
    }
    if (p==Head)  //si es la cabeza
    { if (p->next == NULL)
      { Head = NULL;
        Actual = NULL;
      }
      else
      { if (Actual == Head)
          Actual = Actual->next;
        Head = Head->next;
        Head->prev = NULL;
      }
    }
    else
    { q = p->prev;
      if (Actual == p) Actual = q;
      q->next = p->next;
      if (p->next != NULL)
        p->next->prev = q;
    } //else
    nc--;
    delete p;
  } //if
} //Kill

void* lista::SacarFirst()
{ nodo *p = NULL;
  void *r=NULL;
  if (!Empty())
  { nc--;
    p = Head;
    if (Head->next != NULL)
    { if (Actual==Head)
        Actual = Actual->next;
      Head = Head->next;
      Head->prev = NULL;
      p->next = NULL;
    }
    else
    { Head = NULL;
      Actual = NULL;
    }
    r = p->puntero;
    delete p;
  }
  return r;
}

void* lista::UsarActual()
{ void *p = NULL;
  if (!Empty())
  { if (Actual != NULL)
       p = Actual->puntero;
  }
  return p;
}

void lista::RoundDerechaActual()
{ if (!Empty())
  { if (Actual != NULL)
      if (Actual->next != NULL)
        Actual = Actual->next;
      else
        Actual = Head;
    else
      Actual = Head;
  }
}

void lista::DerechaActual()
{ if (!Empty())
    if (Actual!=NULL)
      Actual = Actual->next;
}

lista::~lista()
{ nodo* p;
  while (!Empty())
  { p = Head;
    Head = Head->next;
    delete p;
  }
}