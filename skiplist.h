#include<iostream>
#include<cstdlib>
#include<cmath>
#include<cstring>
#include<mutex>
#include<fstream>

#define STORE_FILE "../store/dumpFile"

std::mutex mtx;
std::string delimiter=":";

template<typename K,typename V>
class Node
{
  public:
  Node(){}

  Node(K k,V v,int);

  ~Node();

  K get_key() const;
   
  V get_value() const;

  void set_value(V);

  Node<K,V>** forward;

  int node_level;

  private:
  K key;
  V value;
};

template<typename K,typename V>
Node<K,V>::Node(K k,V v,int level)
{
    this->key=k;
    this->value=v;
    this->node_level=level;

    this->forward=new Node<K,V>* [level+1];

    memset(this->forward,0,sizeof(Node<K,V>*)*(level+1));
}

template<typename K,typename V>
Node<K,V>::~Node()
{
    delete [] this->forward;
}

template<typename K,typename V>
K Node<K,V>::get_key() const
{
    return this->key;
}

template<typename K,typename V>
V Node<K,V>::get_value() const
{
    return this->value;
}

template<typename K,typename V>
void Node<K,V>::set_value(V v)
{
    this->value=v;
}

template<typename K,typename V>
class Skiplist
{
private:
    int max_level;
    int skip_now_level;
    Node<K,V>* header;
    int element_count;
    std::ofstream file_writer;
    std::ifstream file_reader;
private:
    void get_key_value_from_string(std::string&s,std::string* key,std::string* value);
    bool is_valid_string(std::string&s);
public:
    Skiplist(int);
    ~Skiplist();
    int get_random_level();
    Node<K,V>* create_node(K,V,int);
    int insert_element(K,V);
    void display_list();
    bool search_element(K);
    void delete_emement(K);
    void dump_file();
    void load_file();
    int size();
    int level(){return skip_now_level;}
};

template<typename K,typename V>
Node<K,V>* Skiplist<K,V>::create_node(K k,V v,int level)
{
          auto N=new Node<K,V>(k,v,level);
          return N;
}


template<typename K,typename V>
int Skiplist<K,V>::insert_element(K k,V v)
{
  mtx.lock();
  Node<K,V>* current=this->header;

  Node<K,V>* update[this->max_level+1];  //保存每一层需要修改的节点
  memset(update,0,sizeof(Node<K,V>*)*(this->max_level+1));


  for(int i=skip_now_level;i>=0;--i)
  {
     while(current->forward[i]!=NULL&&current->forward[i]->get_key()<k)
     {
        current=current->forward[i];
     }

     update[i]=current;
  }

  current=current->forward[0];

  if(current!=NULL&&current->get_key()==k)
  {
    std::cout<<"key:"<<k<<"exists,"<<std::endl;
    mtx.unlock();
    return 1;
  }

  if(current==NULL||current->get_key()!=k)
  {
    int random_level=get_random_level();
    // std::cout<<"Insert Level:"<<random_level<<std::endl;

    if(random_level>skip_now_level)
    {
        for(int i=skip_now_level+1;i<=random_level;++i)
        update[i]=header;
        skip_now_level=random_level;
    }

    Node<K,V>* inserted_node=create_node(k,v,random_level);

    for(int i=0;i<=random_level;++i)
    {
         inserted_node->forward[i]=update[i]->forward[i];
         update[i]->forward[i]=inserted_node;
    }
    element_count++;

    std::cout<<"Successful"<<std::endl;
  }

  mtx.unlock();
  return 0;
}


template<typename K,typename V>
void Skiplist<K,V>::display_list()
{
std::cout<<"****Skip List****"<<"level:"<<skip_now_level<<std::endl;
    for(int i=0;i<=skip_now_level;++i)
    {
        auto node=header->forward[i];
        std::cout<<"Level:"<<i<<":";
        while (node)
        {
            std::cout<<node->get_key()<<":"<<node->get_value()<<"-->";
            node=node->forward[i];
            if(!node)
            std::cout<<"NULL";
        }

        std::cout<<std::endl;
    }
}

//保存只保存最后一层的数据
template<typename K,typename V>
void Skiplist<K,V>::dump_file()
{
    std::cout<<"dump_file"<<std::endl;
    file_writer.open(STORE_FILE);
    Node<K,V>* node=this->header->forward[0];
    while (node!=NULL)
    {
       file_writer<<node->get_key()<<":"<<node->get_value()<<"\n";
       node=node->forward[0];
    }
    
    file_writer.flush();
    file_writer.close();

    return;
}

template<typename K,typename V>
void Skiplist<K,V>::load_file()
{
    std::cout<<"load_file___"<<std::endl;
    file_reader.open(STORE_FILE);

    std::string line;
    std::string* key=new std::string();
    std::string* value=new std::string();

    while (getline(file_reader,line))
    {
        get_key_value_from_string(line,key,value);
        if(key->empty()||value->empty())
        continue;
        insert_element(stoi(*key),*value);
    }
   
   file_reader.close();
}

template<typename K,typename V>
int Skiplist<K,V>::size()
{
    return element_count;
}

template<typename K,typename V>
void Skiplist<K,V>::get_key_value_from_string(std::string&s,std::string* key,std::string* value)
{
    if(!is_valid_string(s))
    return;

    *key=s.substr(0,s.find(delimiter));
    *value=s.substr(s.find(delimiter)+1,s.length());
}

template<typename K,typename V>
bool Skiplist<K,V>::is_valid_string(std::string&s)
{
    if(s.empty())
    return false;

    if(s.find(delimiter)==std::string::npos)
    return false;

    return true;
}

template<typename K,typename V>
void Skiplist<K,V>::delete_emement(K key)
{
  mtx.lock();
  Node<K,V>* current=this->header;

  Node<K,V>* update[this->max_level+1];
  memset(update,0,sizeof(Node<K,V>*)*(this->max_level+1));

  for(int i=skip_now_level;i>=0;--i)
  {
    while(current->forward[i]!=NULL&&current->forward[i]->get_key()<key)
    current=current->forward[i];

    update[i]=current;
  }
  
  current=current->forward[0];
  if(current!=NULL&&current->get_key()==key)
  {
    for(int i=0;i<=skip_now_level;++i)
    {
        if(update[i]->forward[i]->get_key()!=current->get_key())
        break;

        update[i]->forward[i]=current->forward[i];

    }

    //删除以后某层空了需要减少一层
    while (skip_now_level>0&&this->header->forward[skip_now_level]==NULL)
    {
        --skip_now_level;
    } 

    std::cout<<"successful delete:"<<key<<std::endl;
    --element_count;
  }
  mtx.unlock();
  return;
}

//查找举例   4
/*
1                                       5
|
|
v
1--------------->3                      5
                 |
                 |
                 V
1       2        3          4           5
然后判断3后面节点是否等于4
*/


template<typename K,typename V>
bool Skiplist<K,V>::search_element(K key)
{
    // std::cout<<"Search_element:"<<key<<std::endl;

    Node<K,V>* current=this->header;

    for(int i=skip_now_level;i>=0;--i)
    {
        while(current->forward[i]&&current->forward[i]->get_key()<key)
        current=current->forward[i];
    }

    current=current->forward[0];

    if(current!=NULL&&current->get_key()==key)
    {
    std::cout<<"Found:"<<"key:"<<key<<",value:"<<current->get_value()<<std::endl;
    return true;
    }

    std::cout<<"Not Found:"<<key<<std::endl;
    return false;
}

template<typename K,typename V>
Skiplist<K,V>::Skiplist(int Max_levle)
{
this->max_level=Max_levle;
this->skip_now_level=0;
this->element_count=0;
K k;
V v;
this->header=new Node<K,V>(k,v,Max_levle);
}

template<typename K,typename V>
Skiplist<K,V>::~Skiplist()
{
    if(file_reader.is_open())
    file_reader.close();

    if(file_writer.is_open())
    file_writer.close();

    delete header;
}

template<typename K,typename V>
int Skiplist<K,V>::get_random_level()
{
    int l=0;

    while(rand()%2==0)  //抛硬币
    ++l;

    l=std::min(l,max_level);

    return l;
    
}