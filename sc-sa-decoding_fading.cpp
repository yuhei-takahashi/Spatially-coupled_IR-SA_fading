#include "sc-sa-decoding.h"

int counta=0;

/* Generate Random */
int random_edge(int slot, RandomNumberGenerator& seed)
{
  return seed.get_random_number(slot-1);
}

/* decide position slot & device */
void structure_edge(vector<Frame>& str_frame, vector<Device>& f_device, RandomNumberGenerator& seed, Channel& ch, int d)
{
  int t;
  int all_device = 0;
  int count=0;
  int slide=0;
  int k=0;
/* ofstream file("exponential_distribution.tsv"); */
  list<int> li;
  list<double> f;
  
  for(int i=0; i<=sc-constant_times; i++)
    {
      for(int j=all_device; j<str_frame[i].device+all_device; j++)
        {
          for(int j=0; j<constant_times; j++)
            {
              k=seed.get_random_number(str_frame[i].time_slot-1);
              li.push_back(k);
              double fc = seed.exponential_number(ch.snr);
              f.push_back(fc);
               /* file << fc << "\t\n"; */
            }
          slide=0;
          list<double>::iterator fr=f.begin();
          for (list<int>::iterator it=li.begin(); it!=li.end(); it++ )
            {
              f_device[j].add_edge(*it, i+slide, *fr);
              str_frame[i+slide].f_slot[*it].add_edge(j, i, *fr);
              slide++;
              fr++;
            }
          li.clear();
          f.clear();
        }
      all_device += str_frame[i].device;
    }
    
   /* file.close(); */
    
}

/* structure frame */
void structure_frame(vector<Frame>& str_frame, double g)
{
  int n_slot=0;
  int n_device=0;
  for(int i=0; i<sc; i++)
    {
      n_device = (int)(g*number_slot);
      str_frame[i].add_time_slot(number_slot);
      str_frame[i].add_device(n_device);
      for(int j=0; j<number_slot; j++) str_frame[i].add_slot();
    }
}

void display(vector<Frame>& sc_frame, vector<Device>& f_device, double g)
{
  /* device */
  cout << "device" << '\n';
  for(int i=0; i<number_slot*g*(sc-(constant_times-1)); i++)
    {
      list<Edge>::iterator d;
      for(d = f_device[i].d_edge.begin(); d != f_device[i].d_edge.end(); d++)
        {
            cout << "device(" << i << ")"<<  " - " <<  "slot:" << d->position << " (" << d->frame << ")" << '\n';
        }
    }

  /* slot */
  cout << "slot" << '\n';
  for(int i=0; i<sc; i++)
    {
      int count = 0;
      for(int j=0; j<sc_frame[i].time_slot; j++)
        {
          list<Edge>::iterator e;
          for(e = sc_frame[i].f_slot[j].s_edge.begin(); e != sc_frame[i].f_slot[j].s_edge.end(); e++)
            {
              cout << i << "_slot(" << count << ")" << " - " << "device:" << e->position << " (" << e->frame << ")" << '\n';
            }
          count++;
        }
    }
  cout << endl;

}

/* spatially-coupled frame */
void spatially_coupled(vector<Frame>& sc_frame, vector<Device>& f_device, double g, RandomNumberGenerator& seed, Channel& ch, int d)
{

  structure_frame(sc_frame, g);

  structure_edge(sc_frame, f_device, seed, ch, d);

/* display(sc_frame, f_device, g); */

}

void cancellation_slot(int device, int slot, int frame, vector<Frame>& sc_frame)
{
  int device_number = device;
  int slot_number = slot;
  int frame_number = frame;

  for(list<Edge>::iterator e = sc_frame[frame_number].f_slot[slot_number].s_edge.begin(); e != sc_frame[frame_number].f_slot[slot_number].s_edge.end(); e++)
    {
      if(e->position == device_number)
        {
          e = sc_frame[frame_number].f_slot[slot_number].s_edge.erase(e);
        }
    }
}


void successive_interference_cancellation(vector<Frame>& sc_frame, vector<Device>& f_device, int d, RandomNumberGenerator& seed, Channel& ch)
{
  int frame_number;	  /* number of frame have slot connected single edge */
  int device_number;  /* number of slot connected single edge */
  int count1=0;
  int count2=0;
  size_t size;
  
  for(int sic=0;;sic++)
    {
      
      count2=count1;

      for(int i=0; i<sc; i++)
        {

        for(int j=0; j<sc_frame[i].time_slot; j++)
            {
              list<Edge>::iterator e = sc_frame[i].f_slot[j].s_edge.begin();
            
              if(sc_frame[i].f_slot[j].s_edge.size() <= M+1 && sc_frame[i].f_slot[j].s_edge.size() > 0)
              {
                  for(int m=sc_frame[i].f_slot[j].s_edge.size(); m>0; m--)
                  {
                      for(list<Edge>::iterator ds = sc_frame[i].f_slot[j].s_edge.begin();ds != sc_frame[i].f_slot[j].s_edge.end();ds++)
                        {
                          double sinr = 0.0;
                            sinr = ds->fading;
                            double ir = 1.0;
                            int times=0;
                            
                          if(sc_frame[i].f_slot[j].s_edge.size() > 1)
                            {
                              for(list<Edge>::iterator is = sc_frame[i].f_slot[j].s_edge.begin(); is != sc_frame[i].f_slot[j].s_edge.end();is++)
                                {
                                  if(is != ds)
                                    {
                                        ir += is->fading;times++;
                                    }
                                }
                            }
                            
                            ch.snr = sinr*pow(ir,-1);
                         
                            /* success decode */
                            if(ch.set_epsilon() < 1)
                            {
                              frame_number = ds->frame;
                              device_number = ds->position;
 
                              if(f_device[device_number].d_edge.size() > 0)
                                {
                                  for(list<Edge>::iterator q = f_device[device_number].d_edge.begin(); q != f_device[device_number].d_edge.end(); q++)
                                    {
                                      cancellation_slot(device_number,q->position, q->frame , sc_frame);
                                    }
                                    f_device[device_number].d_edge.clear();
                                    count1++;
                                }
                                
                              break;
                            }
                        }
                    }
                }
            }
        }
      if((count1-count2) <= 0) break;
    }
}


double per(vector<Frame>& sc_frame, vector<Device>& f_device, int d)
{
    double error=0, per=0, packet_error_ratio=0;
    int total=0, slide=0, device=0;

  for(int i=0; i < (sc-(constant_times-1)); i++)
    {
        total=0;error=0;slide=i*d;
        for(int j=0;j<d;j++){
            if(f_device[slide+j].d_edge.size() > 0)
            {
                error++;
            }
            total++;
        }
        packet_error_ratio+=error*pow(total,-1);
    }
    
    per=packet_error_ratio*pow((sc-(constant_times-1)),-1);
  return per;

}


void file_output(vector<double>& all_error, vector<double>& loss_trans, char c[])
{

  ofstream outputfile(c);
  outputfile << "L " << sc-constant_times+1 << '\n';
  outputfile << "slot " << number_slot << '\n';
  outputfile << "d " << constant_times << '\n';
  outputfile << "G PER " << '\n';
  vector<double>::iterator b = loss_trans.begin();
  for(vector<double>::iterator a = all_error.begin(); a != all_error.end();a++)
    {
      outputfile << *b << " " <<  *a << '\n';
      b++;
    }
  outputfile.close();

}

int main(int argc, char* argv[])
{
  sc = atoi(argv[1]); /* L+d-1 */
  number_slot = atoi(argv[2]); /* M */
  constant_times = atoi(argv[3]); /* d */

  double packet_error_ratio = 0;
  vector<double> all_error;
  double trans=1.25; /* G=N/M */
  vector<double> loss_trans;
  int device = 0;
  RandomNumberGenerator seed;

  for(int t=1;;t++)
    {

      packet_error_ratio = 0;
      device = (int)(trans*number_slot);
        
      for(int i=0; i<ite; i++)
        {
          Channel ch;
          ch.snr = (double)sigma_h2*1/(pow(10,-1*((double)SNR/10)));

          vector<Frame> sc_frame(sc);
          vector<Device> f_device(device*(sc-(constant_times-1)));
 
          spatially_coupled(sc_frame, f_device, trans, seed, ch, device);

          successive_interference_cancellation(sc_frame, f_device, device, seed, ch);

          packet_error_ratio += per(sc_frame, f_device, device);
         
          seed.reset_seed();
        }
       all_error.push_back(packet_error_ratio/ite);
       loss_trans.push_back(trans*((double)(sc-constant_times+1)/sc));
      
       cout << "G"<<trans<<", G* " << trans*((double)(sc-constant_times+1)/sc) << ", PLR " << packet_error_ratio*pow(ite,-1) << '\n';
        
        
       if(trans*((double)(sc-constant_times+1)/sc) < 0) break;
        trans -= 0.05;
    }
  file_output(all_error, loss_trans, argv[4]);
    
  return 0;
}
