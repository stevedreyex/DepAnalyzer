/*
  This is a diophatine equation solver done by 
  Jenq-Kuen Lee  & Tsaur, In-Kuen, Programming Language Lab, 
  Tsing-Hua University.

  The  Extended_Euclid() function is in the program which
  finds u, x, g, where  au+bx=g,  And the diophatine_solver is to solve
  the diophantine equation. 

*/

/*
#include  <sys\types.h>
#include  <sys\timeb.h>
#include  <time.h>             
*/


#include  <stdio.h>
#include  <math.h>


/*
The solution vector is represented as,

    X= X0 + XCoeffT *t;
    y= Y0 + YCoeffT *t;

*/
struct SolEquStru {
  int   is_there_solution;
  int   X0;
  int   XCoeffT;
  int   Y0;
  int   YCoeffT;
}  ;

/*  
The TriStru is used to hold the gcd information.
  Let gcd = GCD(a,b)  ===> a*x+b*y = gcd
*/ 
struct TriStru{
  int   gcd;
  int   x;
  int   y;
} ;


/*  

The main subroutine, given a, b,
return x, y, g, where a*x+b*y =g;

*/
struct TriStru 
Extended_Euclid(  a ,  b)
int a,b;
{
  struct TriStru Tri1,Tri2;
  if (b==0) {
    Tri1.gcd=a;
    Tri1.x=1;
    Tri1.y=0;
    return Tri1;
    }
  Tri2=Extended_Euclid( b , (a%b));
  Tri1.gcd=Tri2.gcd;
  Tri1.x=Tri2.y;
  Tri1.y=Tri2.x-(a/b)*Tri2.y;
  return Tri1;
}

/*
  Solve the diophatine equation by given a,b, and c, where
    a*x+b*y =c

  return

    X= X0 + XCoeffT *t;
    y= Y0 + YCoeffT *t;

*/   
struct SolEquStru 
diophatine_solver(a,b,c)
int a,b,c;
{
  int   k;
  struct TriStru   Triple;
  struct SolEquStru s;

   Triple=Extended_Euclid( a , b );
   printf("a,b,g, x,y= %d %d %d %d %d \n",a,b,Triple.gcd,Triple.x,Triple.y);
   if ((c%Triple.gcd)==0) {
	k=c/Triple.gcd;
	s.X0=Triple.x*k;
	s.XCoeffT=(b/Triple.gcd );
	s.Y0=Triple.y*k;
	s.YCoeffT=((-a)/Triple.gcd );
        s.is_there_solution=1;
      }
  else s.is_there_solution=0;

  return(s);
}  

int  Min( a, b)
int a , b;
{
 if (a<=b) {return a;}
 else {return b;}
}
int  Max( a,  b)
int a,b;
{
 if (a>=b) {return a;}
 else {return b;}
}


void    main()
{

 struct SolEquStru f;
 int a,b,c;

 printf("input: a b c for a *x + b*y =c\n");
 scanf("%d %d %d",&a,&b,&c);
 f = diophatine_solver(a,b,c);

 if (f.is_there_solution) {
   printf("The solution of the diophatine equation %d x + %d y = % d\n",a,b,c);
   printf("X= %d + %d *t\n", f.X0,f.XCoeffT);
   printf("Y= %d + %d *t\n", f.Y0,f.YCoeffT);
 }
 else {
   printf("There is no solution for the diophatine equation %d x + %d y = % d\n",a,b,c);
 }

}


