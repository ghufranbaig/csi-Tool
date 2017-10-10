function [ p ] = INDFT( h,frq )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

h = rand(32,1)+1j+(rand(32,1));
frq = [2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320, 5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640, 5660, 5680, 5700]';
frq = frq*1e6;
alpha = 5;
epsilon = 0.1;

N = length(frq);
deltaF = 5e6;
M = N;
Tau = [1:M]*1/(deltaF*N);



F = [];
for i = 1:N
    f = [];
    for k = 1:M
        f = [f,exp(-1j*2*pi*frq(i)*Tau(k))];
    end
    F = [F;f];%exp(-1j*2*pi*1*1);
end

%F = zeros(N,M);
%for j = 1:M
%    for i = 1:N
%        F(j,i) = ;
%    end
%end

p = rand(M,1)+1j*rand(M,1);
p = p/max(p);

gamma = 1/norm(F)^2;

while (1)    
    p_new = Sparcify(p-(gamma*F'*(F*p - h)),gamma*alpha);
    norm(p_new-p);
    if (norm(p_new-p)<epsilon)
        p=p_new;
        break
    end
    p = p_new;
    %pause(0.2)

end

end

function [ p_ ] = Sparcify( p,t )
N = length(p);
p_ = p;
for i = 1:N
    if (abs(p_(i))<t)
        p_(i)=0;
    else
        p_(i) = p_(i)* (abs(p_(i))-t)/(abs(p_(i)));
    end
end
end
