clear; clc; close all;
screen_ratio=disp_transform(10,10)./10;
xratio=screen_ratio(1);
yratio=screen_ratio(2);
% Read the data from the text file
files = [...
    "timingsLx10Ly10Nx201Ny201Re10dt0_005T5",...
    "timingsLx1Ly1Nx201Ny201Re10dt0_00005T0_005",...
    "timingsLx1Ly1Nx101Ny101Re500dt0_005T5",...
    "timingsLx1Ly1Nx51Ny51Re100dt0_01T10",...
    "timingsLx1Ly1Nx201Ny201Re1000dt0_005T5",...
    ];


for i  = 1:length(files)
    file = files(i);

    % file = "timingsLx1Ly1Nx201Ny201Re10dt0_00005T0_005";
    data = readmatrix(strcat(file,'.txt'));


    % Extract relevant data
    MPI_ranks = data(:,1);
    OMP_threads = data(:,2);
    INIT_time = data(:,3);
    IC_time = data(:,4);
    SOLVE_time = data(:,5);
    FINAL_time = data(:,6);
    Total_time = data(:,7);

    IDX = 5;
    n=2;
    FIGSIZE=[1000,585,757.6666666666665,652.6666666666665];
    FIGSIZE(1) = FIGSIZE(1) *xratio;
    FIGSIZE(2) = FIGSIZE(2) *yratio;
    FIGSIZE(3) = FIGSIZE(3) *xratio;
    FIGSIZE(4) = FIGSIZE(4) *yratio;

    % MPI V Total
    mpitotal = figure('Position',FIGSIZE);
    p1=plot(MPI_ranks(1:IDX-1), Total_time(1:IDX-1), 'r-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Total Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), Total_time(1:IDX-1), n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');



    % OMP V Total
    omptotal = figure('Position',FIGSIZE);
    p2=plot(OMP_threads(IDX:end), Total_time(IDX:end), 'r-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('Open MP Threads');
    ylabel('Total Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), Total_time(IDX:end), n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % MPI V INIT
    mpiinit = figure('Position',FIGSIZE);
    p1=plot(MPI_ranks(1:IDX-1), INIT_time(1:IDX-1), 'b-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Initialising Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), INIT_time(1:IDX-1), n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % OMP V INIT
    ompinit = figure('Position',FIGSIZE);
    p2=plot(OMP_threads(IDX:end), INIT_time(IDX:end), 'b-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('Open MP Threads');
    ylabel('Initialising Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), INIT_time(IDX:end), n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % MPI V IC
    mpiic = figure('Position',FIGSIZE);
    p1=plot(MPI_ranks(1:IDX-1), IC_time(1:IDX-1), 'g-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Writing Initial Conditions Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), IC_time(1:IDX-1), n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % OMP V IC
    ompic = figure('Position',FIGSIZE);
    p2=plot(OMP_threads(IDX:end), IC_time(IDX:end), 'g-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('Open MP Threads');
    ylabel('Writing Initial Conditions Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), IC_time(IDX:end), n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % MPI V SOLVE
    mpisolve = figure('Position',FIGSIZE);
    p1=plot(MPI_ranks(1:IDX-1), SOLVE_time(1:IDX-1), 'k-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Solve Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), SOLVE_time(1:IDX-1), n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % OMP V SOLVE
    ompsolve =figure('Position',FIGSIZE);
    p2=plot(OMP_threads(IDX:end), SOLVE_time(IDX:end), 'k-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('Open MP Threads');
    ylabel('Solve Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), SOLVE_time(IDX:end), n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % MPI V FINAL
    mpifinal =figure('Position',FIGSIZE);
    p1=plot(MPI_ranks(1:IDX-1), FINAL_time(1:IDX-1), 'm-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Write Final Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), FINAL_time(1:IDX-1), n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');


    % OMP V FINAL
    ompfinal =figure('Position',FIGSIZE);
    p2=plot(OMP_threads(IDX:end), FINAL_time(IDX:end), 'm-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('Open MP Threads');
    ylabel('Write Final Time [$s$]');
    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), FINAL_time(IDX:end), n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');

    % MPI V normalise time
    mpinormal =figure('Position',FIGSIZE);
    % normal = Total_time(1:IDX-1)./max(Total_time(1:IDX-1));
    %  p3=plot(MPI_ranks(1:IDX-1), normal, 'rx', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('MPI Ranks');
    ylabel('Normalised Time');

    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(MPI_ranks(1:IDX-1), normal, n); % Second-order polynomial
    % x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    % y_fit = spline(p, x_fit);
    % %plot(x_fit, y_fit, 'r--');

    normal = INIT_time(1:IDX-1)./max(INIT_time(1:IDX-1));
    p4=plot(MPI_ranks(1:IDX-1), normal, 'b-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(MPI_ranks(1:IDX-1), normal, n); % Second-order polynomial
    x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    y_fit = spline(MPI_ranks(1:IDX-1),normal,x_fit);
    %plot(x_fit, y_fit, 'b--');

    normal = IC_time(1:IDX-1)./max(IC_time(1:IDX-1));
    p5=plot(MPI_ranks(1:IDX-1), normal, 'g-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(MPI_ranks(1:IDX-1), normal, n); % Second-order polynomial
    x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    y_fit = spline(MPI_ranks(1:IDX-1),normal,x_fit);
    %plot(x_fit, y_fit, 'g--');

    normal = SOLVE_time(1:IDX-1)./max(SOLVE_time(1:IDX-1));
    p6=plot(MPI_ranks(1:IDX-1), normal, 'k-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(MPI_ranks(1:IDX-1), normal, n); % Second-order polynomial
    x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    y_fit = spline(MPI_ranks(1:IDX-1),normal,x_fit);
    %plot(x_fit, y_fit, 'k--');

    normal = FINAL_time(1:IDX-1)./max(FINAL_time(1:IDX-1));
    p7=plot(MPI_ranks(1:IDX-1), normal, 'm-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(MPI_ranks(1:IDX-1), normal, n); % Second-order polynomial
    x_fit = min(MPI_ranks(1:IDX-1)):0.1:max(MPI_ranks(1:IDX-1));
    y_fit = spline(MPI_ranks(1:IDX-1),normal,x_fit);
    %plot(x_fit, y_fit, 'm--');

    % Get the current y-axis limits
    current_ylim = ylim;

    % Set the lower limit to 0 and keep the upper limit unchanged
    ylim([0, current_ylim(2)]);
    legend([p4,p5,p6,p7],["Initialise","Write Initial","Solve","Write Final"])


    % OMP V normalise time
    ompnormal =figure('Position',FIGSIZE);
    % normal = Total_time(IDX:end)./max(Total_time(IDX:end));
    % p3=plot(OMP_threads(IDX:end), normal, 'r-x', 'MarkerSize', 10); % Red cross markers with increased size
    xlabel('OMP threads');
    ylabel('Normalised Time');

    hold on;
    % Add curved line of best fit (second-order polynomial)
    % %p=polyfit(OMP_threads(IDX:end), normal, n); % Second-order polynomial
    % x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    % y_fit = spline(OMP_threads(IDX:end),normal,x_fit);
    % %plot(x_fit, y_fit, 'r--');

    normal = INIT_time(IDX:end)./max(INIT_time(IDX:end));
    p4=plot(OMP_threads(IDX:end), normal, 'b-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(OMP_threads(IDX:end), normal, n); % Second-order polynomial
    x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    y_fit = spline(OMP_threads(IDX:end),normal,x_fit);
    %plot(x_fit, y_fit, 'b--');

    normal = IC_time(IDX:end)./max(IC_time(IDX:end));
    p5=plot(OMP_threads(IDX:end), normal, 'g-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(OMP_threads(IDX:end), normal, n); % Second-order polynomial
    x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    y_fit = spline(OMP_threads(IDX:end),normal,x_fit);
    %plot(x_fit, y_fit, 'g--');

    normal = SOLVE_time(IDX:end)./max(SOLVE_time(IDX:end));
    p6=plot(OMP_threads(IDX:end), normal, 'k-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(OMP_threads(IDX:end), normal, n); % Second-order polynomial
    x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    y_fit = spline(OMP_threads(IDX:end),normal,x_fit);
    %plot(x_fit, y_fit, 'k--');

    normal = FINAL_time(IDX:end)./max(FINAL_time(IDX:end));
    p7=plot(OMP_threads(IDX:end), normal, 'm-x', 'MarkerSize', 10); % Red cross markers with increased size
    hold on;
    % Add curved line of best fit (second-order polynomial)
    %p=polyfit(OMP_threads(IDX:end), normal, n); % Second-order polynomial
    x_fit = min(OMP_threads(IDX:end)):0.1:max(OMP_threads(IDX:end));
    y_fit = spline(OMP_threads(IDX:end),normal,x_fit);
    %plot(x_fit, y_fit, 'm--');

    current_ylim = ylim;
    ylim([0,current_ylim(2)]);
    legend([p4,p5,p6,p7],["Initialise","Write Initial","Solve","Write Final"])




    prettier();

 
    figure(mpitotal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVTOTAL"),"png");
    figure(omptotal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVTOTAL"),"png");
    figure(mpiinit);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVINIT"),"png");
    figure(ompinit);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVINIT"),"png");
    figure(mpiic);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVIC"),"png");
    figure(ompic);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVIC"),"png");
    figure(mpisolve);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVSOLVE"),"png");
    figure(ompsolve);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVSOLVE"),"png");
    figure(mpifinal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVFINAL"),"png");
    figure(ompfinal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVFINAL"),"png");
    figure(mpinormal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___MPIVNORMAL"),"png");
    figure(ompnormal);
    currxlim = xlim;
    xlim([1,currxlim(2)]);
    saveas(gcf,strcat("../Images/",file,"___OMPVNORMAL"),"png");
    
    close all;
end
