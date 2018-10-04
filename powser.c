// 09/24/2018 02:14:17 PM 
// An implementation of lab4 challenge : power series calculator

#include <inc/lib.h>


typedef struct _rat {
	int32_t den;
	int32_t num;
} rat;

int16_t gcd(int32_t i, int32_t j) {
	if(i < 0)
		return gcd(-i, j);
	else if(i == 0)
		return j;
	return gcd(j%i, i);
}


rat mul(rat a, rat b)
{
	rat answer;
	int32_t num;
	int32_t den;
	int32_t gcd_res;
	num = a.num*b.num;
	den = a.den*b.den;
	gcd_res = gcd(num, den);
	answer.num = num / gcd_res;
	answer.den = den / gcd_res;	
	return answer;
}

void Mul(envid_t Out)
{
	rat answer;
	rat a;
	rat b; 
	envid_t channelA = ipc_recv(chanOut, NULL, 0, 0);
	envid_t channelB = ipc_recv(chanOut, NULL, 0, 0);
	while(1) {
		a.num = ipc_recv(chanA, NULL, 0, 0);
		a.den = ipc_recv(chanA, NULL, 0, 0);
		b.num = ipc_recv(chanB, NULL, 0, 0);
		b.den = ipc_recv(chanB, NULL, 0, 0);
		answer = mul(a, b);	
		ipc_send(chanOut, answer.num, 0, 0);	
		ipc_send(chanOut, answer.den, 0, 0);	
	}
	return;	
}

void Integ(rat first, envid_t in)
{

	static int i = 0;
	rat a, b, result;
	while(1) {
			if(i==0){
				ipc_send(in, first.num, 0, 0);	
				ipc_send(in, first.den, 0, 0);	
			}
			else {	
				a.num = ipc_recv(-1, NULL, 0, 0);
				a.den = ipc_recv(-1, NULL, 0, 0);
				b.num = 1;
				b.den = i;
				result = mul(a, b);			
				ipc_send(in, result.num, 0, 0);
				ipc_send(in, result.den, 0, 0);
			}
			i++;	
	}	
}

//integrate cos to calculate sin
//sinx = integral cosx
void Sin(envid_t master)
{
	envid_t cos = ipc_recv(master, NULL, 0, 0);
	envid_t cos_integ;
	static int i = 0;
	rat a, b, result;
	rat zero = {.num = 0, .den = 1};
	envid_t integ;  
	envid_t myid = sys_getenvid();
	int32_t temp;
	//start  the integral process 
	if ((integ = fork()) < 0)
		panic("fork: %e", integ);
	if (integ == 0) {
		Integ(zero, myid);
		return;
	}	
	cos_integ = ipc_recv(cos, NULL, 0, 0);
	ipc_send(cos, integ, 0, 0);			
	
	while(1){	
		temp = ipc_recv(integ, NULL, 0, 0); //receive the num
		ipc_send(cos_integ, temp, 0, 0);
		ipc_send(master, temp, 0, 0);
		temp = ipc_recv(integ, NULL, 0, 0); //receive the den
		ipc_send(cos_integ, temp, 0, 0);
		ipc_send(master, temp, 0, 0);
		i++;	
	}	
}

//integrate sin to calculate cos
//cosx = 1 - integral sinx
void Cos(envid_t master)
{
	static unsigned int i = 0;
	rat a, b, result;
	int temp;
	envid_t sin = ipc_recv(master, NULL, 0, 0);
	envid_t integ; 
	envid_t sin_integ;
	envid_t send_process_id;
	rat one = {.num = 1, .den = 1};
	
	envid_t myid = sys_getenvid();

	//start  the integral process
	if ((integ = fork()) < 0)
		panic("fork: %e", integ);
	if (integ == 0) {
		Integ(one, myid);
		return;
	}

	ipc_send(sin, integ, 0, 0);
	sin_integ = ipc_recv(sin, NULL, 0, 0);

	temp = ipc_recv(integ, NULL, 0, 0);
	temp = ipc_recv(integ, NULL, 0, 0);
	ipc_send(sin_integ, 1, 0, 0);
	ipc_send(sin_integ, 1, 0, 0);			
	while(1) {
		a.num = ipc_recv(integ, NULL, 0, 0);//receive the num
		a.den = ipc_recv(integ, NULL, 0, 0);//receive the den
		b.num = -1;
		b.den = 1;
		result = mul(a, b);	
		ipc_send(sin_integ, result.num, 0, 0);
		ipc_send(sin_integ, result.den, 0, 0);
	}	
}

void
umain(int argc, char **argv)
{
	int i, sin, cos;
	envid_t myid = sys_getenvid();

	if ((sin = fork()) < 0)
		panic("fork: %e", sin);
	if (sin == 0) {
		Sin(myid);
		return;
	}
	
	if ((cos = fork()) < 0)
		panic("fork: %e", cos);
	if (cos == 0) {
		Cos(myid);
		return;
	}

	ipc_send(sin, cos, 0, 0);
	ipc_send(cos, sin, 0, 0);
	
	for (i = 1; i<20; i++) {
		cprintf("%d/", ipc_recv(sin, NULL, 0, 0));
		cprintf("%d\n", ipc_recv(sin, NULL, 0, 0));
	}
}

