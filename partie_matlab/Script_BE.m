close all;

%Définition des constantes
%echantillonage
Fe = 2*10^3; %Hz
Te = 1/Fe;

%gain pour le LEM et la shunt
Gain_LEM = 104*10^-3; %V/A
%R_shunt = 2*10^-3; % Ohm
Gain_Shunt = 0.12; % V/A
%f_i = 1/ (tau_i); %calculé
U_bat = 24; %V du moteur
R = 1;%ohm du moteur
L = 2*10^-3;%Henry du moteur
tau_i_lem = (2*U_bat*Gain_LEM)/(2*pi*200); %calculé
tau_i_shunt = (2*U_bat*Gain_Shunt)/(2*pi*200); %calculé


%Gains de retour
K_LEM = L/(R*tau_i_lem); %gain pour la FTBO Hall en V/V
K_Shunt= L/(R*tau_i_shunt); % gain pour la FTBO shunt en V/V

%Définitions des FT Sans filtre

FTBO_Hall = K_LEM*2*U_bat*Gain_LEM* tf([tau_i_lem 1], [tau_i_lem^2 tau_i_lem 0]);
FTBO_Shunt = K_Shunt*2*U_bat*Gain_Shunt* tf([tau_i_shunt 1], [tau_i_shunt^2 tau_i_shunt 0]);

% %Test des FT sans filtre
% figure;
% subplot(2,1,1);
% margin(FTBO_Hall);
% title('FTBO Hall');
% 
% subplot(2,1,2);
% margin(FTBO_Shunt);
% title('FTBO Shunt');

%Définitions des filtres

%Filtre d'ordre 1 pour Effet Hall
R_ordre1=5100;%ohm
C=22*10^-9;%Farad
tau_ordre1 = R_ordre1*C;
num_ordre1 = [1]; %numerateur du filtre ordre1
denum_ordre1 = [tau_ordre1 1]; %denumerateur du filtre ordre1
filtre_ordre1=tf(num_ordre1,denum_ordre1);

%Filtre ordre 2+1 Hall et Shunt
%ordre 2
omega_sallen = 5.96*10^3;%
tau_sallen = 1/omega_sallen;
m_sallen = sqrt(2)/2;
num_sallen = [1];
denum_sallen = [tau_sallen^2 tau_sallen*m_sallen 1];
filtre_sallen = tf(num_sallen,denum_sallen);
%ordre 1
f_RC = 102*10^3;%Herz
tau_RC = 1/(2*pi*f_RC);
num_RC = [1];
denum_RC = [tau_RC 1];
filtre_RC = tf(num_RC, denum_RC);


% Test des filtres
% figure;
% 
% subplot(1,3,1);
% margin(filtre_RC);
% title('Filtre RC');
% 
% subplot(1,3,2);
% margin(filtre_sallen);
% title('Filtre Sallen Key');
% 
% subplot(1,3,3);
% margin(filtre_sallen*filtre_RC);
% title('Filtre Sallen Key et RC');


%% Test des systèmes en boucle ouverte avec filtres
% figure;
% 
%FTBO_Hall avec le filtre ordre1
% FTBO_Hall_filtre1=FTBO_Hall*filtre_ordre1;
% subplot(1,3,1);
% margin(FTBO_Hall_filtre1);
% title('Margin FTBO Hall filtre ordre 1');
% % 
% %FTBO_Hall avec le filtre ordre 2+1
% FTBO_Hall_filtre2p1=FTBO_Hall*filtre_sallen*filtre_RC;
% subplot(1,3,2);
% margin(FTBO_Hall_filtre2p1);
% title('Margin FTBO Hall filtre 2+1');
% 
% %FTBO_Shunt avec le filtre ordre 2+1
% FTBO_Shunt_filtre2p1=FTBO_Shunt*filtre_sallen*filtre_RC;
% subplot(1,3,3);
% margin(FTBO_Shunt_filtre2p1);
% title('Margin FTBO shunt filtre 2+1');

%FT Moteur
tau_i=tau_i_lem;
f_moteur=tf(2*U_bat,[tau_i 1]);
% figure;
% margin(f_moteur);
% title('moteur');


%Récuperation de la sortie de la sim simulink

Gain_Capteur = Gain_LEM;
K_Corr = K_LEM;
num_filtre = num_ordre1;
denum_filtre = denum_ordre1;
figure;
out = sim("simulink_trot_continu.slx");
outd = sim("simulink_trot_discret.slx");
sortie = out.simout;
retour = out.retour;
corr = out.correcteur;
sortie_d = outd.simout;
retour_d = outd.retour;
corr_d = outd.correcteur;

subplot(3,1,1);
plot(sortie);
hold on;
plot(sortie_d,'--r');
grid;
title('Sortie du système');
hold off; 

subplot(3,1,2);
plot(retour);
hold on;
plot(retour_d,'--r');
grid;
title('Retour');
hold off;

subplot(3,1,3);
plot(corr);
hold on;
plot(corr_d,'--r');
grid;
title('Correcteur');
hold off;
%%Discretisation du correcteur
f_corr =  tf([K_Corr*tau_i 1], [tau_i 0]);%ft du correcteur

f_corr_d = c2d(f_corr, Te);%ft du correcteur discrète
figure;
step(f_corr,'-b',f_corr_d,'--r');
figure;
margin(f_corr,'-b');
figure;
margin(f_corr_d,'--r');
