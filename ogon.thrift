typedef i64     DWORD_RPC;
typedef i64     SCARDCONTEXT_RPC;
typedef i64     LONG_RPC;
typedef i64     LPDWORD_RPC;
typedef string  LPSTR_RPC;
typedef string  LPCSTR_RPC;

struct return_ec {
  1: LONG_RPC           retValue,
  2: SCARDCONTEXT_RPC   cardContext
}

struct return_lr {
  1: LONG_RPC         retValue,
  2: LPSTR_RPC        mszReaders, 
  3: LPDWORD_RPC      pcchReaders
}

service ogon {
   return_ec EstablishContext(1: DWORD_RPC dwScope)
   return_lr ListReaders(1: SCARDCONTEXT_RPC hContext);
}





