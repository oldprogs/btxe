
/******************************
 *    Pointer Double List     *
 *          Template          *
 *                            *
 *         headerfile         *
 *                            *
 * written by Michael Reinsch *
 *                            *
 ******************************/


template <class Type>
class PntDListEntry
{

 public:

  Type                  *entry;
  PntDListEntry<Type>   *prev_entry;
  PntDListEntry<Type>   *next_entry;
  UINT32                 id;

  inline                 PntDListEntry ( Type *, UINT32 = 0 );

};


template <class Type>
inline PntDListEntry<Type> :: PntDListEntry ( Type *tmp, UINT32 num )
{
  entry      = tmp;
  prev_entry = NULL;
  next_entry = NULL;
  id         = num;
}



template <class Type>
class PntDList
{

 private:

  PntDListEntry<Type>    *list;
  UINT32                  num_entries;

  inline void             Renumber       ( void );

 public:

  inline                  PntDList     ( void );
  inline                 ~PntDList     ( void );

  inline UINT32           Entries        ( void );

  inline UINT32           Add            ( Type * );
  inline BOOLEAN          Remove         ( Type * );

  inline Type            *operator[]     ( UINT32 );
  inline Type            *GetEntry       ( void );

  inline BOOLEAN          JmpFirst       ( void );
  inline BOOLEAN          JmpLast        ( void );
  inline BOOLEAN          JmpNext        ( void );
  inline BOOLEAN          JmpPrev        ( void );

  inline BOOLEAN          Find           ( Type * );

};


template <class Type>
inline PntDList<Type> :: PntDList ( void )
{
  list        = NULL;
  num_entries = 0;
}


template <class Type>
inline PntDList<Type> :: ~PntDList ( void )
{
  while ( list != NULL )
  {
    Remove ( list->entry );
  }
}


template <class Type>
inline UINT32 PntDList<Type> :: Entries ( void )
{
  return num_entries;
}


template <class Type>
inline UINT32 PntDList<Type> :: Add ( Type *tmp )
{
  PntDListEntry<Type> *new_entry = new PntDListEntry<Type> ( tmp, num_entries );

  if ( list == NULL )
  {
    list = new_entry;
  }
  else
  {
    JmpLast ( );
    new_entry->prev_entry = list;       // prev_entry for new_entry is list
    list->next_entry      = new_entry;  // next_entry for list is new_entry
  }

  num_entries++;
  return num_entries;
}


template <class Type>
inline BOOLEAN PntDList<Type> :: Remove ( Type *tmp )
{
  PntDListEntry<Type> *del_entry;

  if ( Find ( tmp ) )
  {
    del_entry = list;

    if ( del_entry->prev_entry )
    {
      list             = del_entry->prev_entry;
      list->next_entry = del_entry->next_entry;
    }

    if ( del_entry->next_entry )
    {
      list             = del_entry->next_entry;
      list->prev_entry = del_entry->prev_entry;
    }

    if ( list == del_entry )
    {
      list = NULL;
    }

    delete del_entry;
    num_entries--;
    Renumber ();

    return true;
  }

  return false;
}


template <class Type>
inline Type *PntDList<Type> :: operator[] ( UINT32 id )
{
  if ( id < num_entries )
  {
    while ( list->id != id )
    {
      if ( list->id < id )
        JmpNext();
      else
        JmpPrev();
    }

    return list->entry;
  }

  return NULL;
}


template <class Type>
inline Type *PntDList<Type> :: GetEntry ( void )
{
  if ( list != NULL )
    return list->entry;
  else
    return NULL;
}


template <class Type>
BOOLEAN PntDList<Type> :: JmpFirst ( void )
{
  if ( list != NULL )
  {
    while ( JmpPrev() );
    return true;
  }

  return false;
}


template <class Type>
BOOLEAN PntDList<Type> :: JmpLast ( void )
{
  if ( list != NULL )
  {
    while ( JmpNext() );
    return true;
  }

  return false;
}


template <class Type>
BOOLEAN PntDList<Type> :: JmpNext ( void )
{
  if ( list != NULL )
    if ( list->next_entry )
    {
      list = list->next_entry;
      return true;
    }

  return false;
}


template <class Type>
BOOLEAN PntDList<Type> :: JmpPrev ( void )
{
  if ( list != NULL )
    if ( list->prev_entry )
    {
      list = list->prev_entry;
      return true;
    }
  
  return false;
}


template <class Type>
inline BOOLEAN PntDList<Type> :: Find ( Type *tmp )
{
  if ( list != NULL )
  {
    UINT16 i = JmpFirst ( );

    while ( i )
    {
      if ( list->entry == tmp ) return true;
      i = JmpNext ( );
    }
  }

  return false;
}


template <class Type>
inline void PntDList<Type> :: Renumber ( void )
{
  if ( list != NULL )
  {
    UINT32 num = 0;
    UINT16 i   = JmpFirst ( );

    while ( i )
    {
      list->id = num++;
      i        = JmpNext ( );
    }
  }
}


