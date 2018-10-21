/**
  * FileName: VSemMutex.h
  * Author: Created by tyreezhang
  * History:
  */


#include "Exception.h"
#include "Lock.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
  
namespace Util
{

/**
 * system v –≈∫≈¡ø
 */
class VSemMutex
{
public:
  typedef LockP<VSemMutex> PLock;
  
  VSemMutex(int key, int perm = 0666);
  ~VSemMutex();

  void lock() const ;
  bool tryLock() const;
  
  void unLock() const;

protected:
  mutable int m_semID;
};

}


