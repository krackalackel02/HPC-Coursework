function [USER_DIMENSIONS] = disp_transform(x,y)
% PANELGEN() is a function to generate and discretize airfoil
% 
% *VARIABLES:*
% 
% * *INPUT_DIMENSIONS* - Input dimensions of x and y to be transformed
% * *my_pc_res* - Screen resolution of computer program was wrote on
% * *user_pc_res* - User's screen resolution
% * *USER_DIMENSIONS* - Transformed version on input dimensions
% 
% 
% Using the relative scale factor produced by user computer over original
% computer we can fins new dimensions to use for figures for same visual
% output

my_pc_res = [1,1,2560,1440];
set(0,'units','pixels');
user_pc_res = get(0,'ScreenSize');
USER_DIMENSIONS(1) = x*user_pc_res(3)/my_pc_res(3);
USER_DIMENSIONS(2) = y*user_pc_res(4)/my_pc_res(4);
end