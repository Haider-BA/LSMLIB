%
% File:        computeDistanceFunction3d.m
% Copyrights:  (c) 2005 The Trustees of Princeton University and Board of
%                  Regents of the University of Texas.  All rights reserved.
%              (c) 2009 Kevin T. Chu.  All rights reserved.
% Revision:    $Revision$
% Modified:    $Date$
% Description: MATLAB script to process the data file generated by
%              computeDistanceFunction3d
%

% geometric parameters
radius = 0.5;

% load data file
data = load('computeDistanceFunction3d.dat');
phi = data(:,1);
phi_init = data(:,2);

% compute the number of grid points
num_gridpts = length(phi);
N = round(num_gridpts^(1/3));

% reshape to 3D array
phi = reshape(phi,N,N,N); phi = permute(phi,[2 1 3]);
phi_init = reshape(phi_init,N,N,N); phi_init = permute(phi_init,[2 1 3]);

% generate mesh
dx = 2/(N-1);
X = -1:dx:1;
Y = X;
Z = X;

% show zero level set
figure(1); clf;
isosurface(X,Y,Z,phi,0);
axis([-1 1 -1 1 -1 1]);
axis square;
xlabel('x'); ylabel('y'); zlabel('z');

% show slices of distance function
figure(2); clf;
slice(X,Y,Z,phi,[0],[0],[0]);
title('distance function');
color_axis = caxis; % get color axis to use in other plot
colorbar;
axis square;
xlabel('x'); ylabel('y'); zlabel('z');

% show slices of original level set function
figure(3); clf;
slice(X,Y,Z,phi_init,[0],[0],[0]);
title('original level set function');
caxis(color_axis);  % set color axis to be the same as the distance function
                    % plot
colorbar;
axis square;
xlabel('x'); ylabel('y'); zlabel('z');

% do some checks to see if phi is a distance function
figure(4); clf;
plot(X,phi(ceil(N/2),:,ceil(N/2)),'b');  % X coord comes second in MATLAB
hold on;
plot([-radius radius],[0 0],'ro');
