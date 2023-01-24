#include<iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

class MyCdev
{
   static constexpr char *DEVNAME = "/dev/MyCdev";
   static constexpr int SIZE = 512;

   public:
      MyCdev();
      ~MyCdev();

      void Read();
      void Write();
   
   private:
      int32_t m_fd = -1;
#ifdef DYNAMIC_ARRAY
      uint8_t *m_buff = nullptr;
#else
      uint8_t m_buff[SIZE] = {0};
#endif
};

MyCdev::MyCdev()
{
   m_fd = open(DEVNAME, O_RDWR);
   if(m_fd < 0)
   {
      std::cout << "Unable to open file - " << DEVNAME << " errno: " << strerror(errno) << "\n";
   }

#ifdef DYNAMIC_ARRAY
   m_buff = new uint8_t(SIZE);
#endif
}

MyCdev::~MyCdev()
{
#ifdef DYNAMIC_ARRAY
   delete[] m_buff;
#endif 
   if(m_fd>0)
   {
      close(m_fd);
      std::cout << "closed device file\n";
   }

}

void MyCdev::Read()
{
   //std::cout << ">>> Read: Address of m_buff: " << &m_buff[0] << "\n";
   printf(">>> Read: Address of m_buff: %p\n", &m_buff[0]);
   fflush(stdout);

   memset(m_buff, 0x11, SIZE);

   ssize_t ret = read(m_fd, m_buff, 10);
   std::cout << "Read Return: " << ret << "\n";
   
   for(int index = 0; index < 10; index++)
   {
      std::cout << m_buff[index] << " ";
   }
   
   std::cout << "\n";

   return;
}

void MyCdev::Write()
{
   std::cout << ">>> Write\n";
   
   memset(m_buff, 0xff, 10);
 
   ssize_t ret = write(m_fd, m_buff, 10);

   std::cout << "Write return: " << ret << "\n";

   return;
}

int main(int argc, char *argv[])
{
   MyCdev dev;

   dev.Read();

   //dev.Write();

}
