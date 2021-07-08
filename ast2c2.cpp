a =(1:30).sqrt


If LHS = RHS, this is a copy operation. Eval RHS. Instead of doing all over again, take advantage of Sig


Compute(RHS);
Now Sig is ready.

if Sig.type is vector


double *_b = RHS2LHS();

double *_b = Compute2LHS(RHS)
{
    Compute(RHS);
    double *_b = new double[Sig.nSamples];
    memcpy(_b, &Sig.buf, Sig.bufBlockSize * Sig.nSamples);
    return _b;
}


A statement like this    b = a([3 5 9])+2
a should be ready

double *_b = Extract2LHS(RHS)
{
    // retrieve
    Compute(RHS);
    // Now Sig indicates index
}
y=1:10000
for k=1:10000,     a = k * -1,     y(k) = -k*10, end

