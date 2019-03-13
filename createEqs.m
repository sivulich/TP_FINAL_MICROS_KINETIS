function [] = createEqs(gain,step)
%CREATEEQS Summary of this function goes here
%   Detailed explanation goes here
str='{';
for i=-gain:1:gain

    sosl = iirparameq(4,i,750/48000,900/48000);
    sosm = iirparameq(4,i,3050/48000,5900/48000);
    sosh = iirparameq(4,i,8500/48000,15000/48000);
    hold all;
    freqz(sosl)
    hold all;
    freqz(sosm);
    hold all;
    freqz(sosh);
    temp=strcat('{',sprintf('{%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f},',sosl(1,1),0,sosl(1,2),sosl(1,3),-sosl(1,5),-sosl(1,6),sosl(2,1),0,sosl(2,2),sosl(2,3),-sosl(2,5),-sosl(2,6)),...
    sprintf('{%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f},',sosm(1,1),0,sosm(1,2),sosm(1,3),-sosm(1,5),-sosm(1,6),sosm(2,1),0,sosm(2,2),sosm(2,3),-sosm(2,5),-sosm(2,6)),...
    sprintf('{%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f}',sosh(1,1),0,sosh(1,2),sosh(1,3),-sosh(1,5),-sosh(1,6),sosh(2,1),0,sosh(2,2),sosh(2,3),-sosh(2,5),-sosh(2,6)));
    if(i~=gain)
        temp=strcat(temp,'},\n');
    else
        temp=strcat(temp,'}\n');
    end
    str=strcat(str,temp);
end
str=strcat(str,'}\n');
fprintf(str);
end

