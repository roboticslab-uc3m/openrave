%% Script for simulating a laser range finder and interacting with matlab
%% load a robot with a sensor mounted on it
function testlasersensor(render)

more off; % turn off output paging
addopenravepaths()

if( ~exist('render','var') )
    render = [0];
end

orEnvLoadScene('data/testwamlaser.env.xml',1); % reset the scene
robots = orEnvGetRobots();
robotid = robots{1}.id;

% to turn on the rendering, send a command to the sensor
for i = 1:length(render)
    orRobotSensorSend(robotid, render(i), 'show','1');
end

sensorindex = 0;

while(1)
% get the laser data
data = orRobotSensorGetData(robotid, sensorindex);

% get the indices of the colliding bodies for each laser point
collidingbodies = sscanf(orRobotSensorSend(robotid, sensorindex, 'collidingbodies'), '%f');

disp(sprintf('found %d laser points, %f%% are hitting objects', ...
    size(data.laserrange,2), sum(collidingbodies>0)/length(collidingbodies)*100));
distances = sqrt(sum(data.laserrange.^2));
disp(sprintf('min and max measurements are %f and %f', min(distances), max(distances)));
pause(0.2);
end
