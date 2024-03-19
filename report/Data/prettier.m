function prettier()
%%%%CHOOSE DESIGN
LINEWIDTH = 3;
MARKERSIZE = 10;
LEGENED_FONTSIZE = 15;
AXES_FONTSIZE = 20;



all_figures = findobj('Type', 'figure');
for i = 1:length(all_figures)
    % Select the figure by its handle
    figure(all_figures(i));
    fig = gcf;
    graph_pretty(fig,LINEWIDTH,MARKERSIZE,LEGENED_FONTSIZE,AXES_FONTSIZE)
end
end


function  graph_pretty (fig,LINEWIDTH,MARKERSIZE,LEGENED_FONTSIZE,AXES_FONTSIZE)
% Get the handle of the current figure
% fig = gcf;

% Get the handles of all the axes in the figure
axesHandles = findall(fig, 'type', 'axes');

% Iterate through each axes
for i = 1:numel(axesHandles)
    % Get the handle of the current axes
    a = axesHandles(i);
    a.FontSize = LEGENED_FONTSIZE;
    % a.Legend.FontSize = 15;
    % a.Legend.Location = 'Best';

    a.Title.FontWeight = 'Bold';
    a.Title.Interpreter = 'latex';
    a.YLabel.FontSize = AXES_FONTSIZE;
    a.YLabel.FontWeight = 'Bold';
    a.YLabel.Interpreter = 'latex';
    a.XLabel.FontSize = AXES_FONTSIZE;
    a.XLabel.FontWeight = 'Bold';
    a.XLabel.Interpreter = 'latex';
    box on;
    a.LineWidth = LINEWIDTH/2;
    % Loop through the children elements and check their names
    for i = 1:height(a.Children)
        if strcmp(a.Children(i).Type, 'line')
            a.Children(i).LineWidth = LINEWIDTH;
            a.Children(i).MarkerSize = MARKERSIZE;
        end
    end
    a.XMinorGrid = 'on';
    a.XMinorTick = 'on';
    a.YMinorGrid = 'on';
    a.YMinorTick = 'on';
    a.XTickLabelMode = 'auto';



end
for i =1:height(fig.Children(1))
    leg = fig.Children(1);
    if isprop(leg,"Location")
        leg.Location = 'best';
        leg.FontSize = LEGENED_FONTSIZE;
        leg.FontWeight = 'Bold';
        leg.Interpreter = 'latex';
    end



end

plot_num = flip(fig.Children(1).Children);
plotsavail = [1:1:height(plot_num)];

% % % % % % choose which plots to keep
% plotstokeep = [1];
%
%
%
% match_plot = ~ismember(plotsavail, plotstokeep);
% match_plot = match_plot.*plotsavail;
% match_plot = match_plot(match_plot ~= 0);
% for i = match_plot
%     delete(plot_num(i));
% end
%
%
% plotstokeep = flip(plotstokeep);
% prevy = 0;
% for i = plotstokeep
%     plot_num(i).Units = 'normalized';
%     plot_num(i).Position = [0 prevy 1 1/length(plotstokeep)];
%     prevy = prevy + 1/length(plotstokeep);
%
% end

end

