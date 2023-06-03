import React from 'react';
import RoverCarousel from '../components/Home/RoverCarousel';
import ReplayCarousel from '../components/Home/ReplayCarousel';
import '../components/Home/grid_Home.css';
import '../components/grid.css';

const Home = () => {
	// Gets from server: MAC, Nickname, Connection status
	const rovers_list = [
		{MAC: '11:11:11:11:11:11', nickname: 'David2', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle12', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David43', overlayText: 'OFFLINE' },
		{MAC: '11:11:11:11:11:11', nickname: 'David3', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle65', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David15', overlayText: 'OFFLINE' },
		{MAC: '11:11:11:11:11:11', nickname: 'David4', overlayText: 'CONNECT' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle2', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David15', overlayText: 'OFFLINE' },
	  ];

	  const replays_list = [
		{ID: '1', name: 'run1'},
		{ID: '2', name: 'trial'},
		{ID: '3', name: 'test'},
		{ID: '4', name: 'It works!!'},
		{ID: '5', name: "didn't work :("},
		{ID: '6', name: 'aaaaa'},
		{ID: '7', name: 'ababa'},
		{ID: '8', name: 'what'},
		{ID: '9', name: 'wtf'},
	  ];

	  
	

	return (
		<div className="background">
			<div className="wrapper">
				<div className="box Group1_Home">
					Group 1				
				</div>
				<div className="box RoversText_Home">
					Rovers
				</div>
				<div className="box RoverCarousel_Home">
					<RoverCarousel rovers={rovers_list}/>
				</div>
				<div className="box AddButton_Home">

				</div>
				<div className="box ReplaysText_Home">
					Replays
				</div>
				<div className="box ReplayCarousel_Home">
					<ReplayCarousel replays={replays_list} />
				</div>
			</div>
		</div>
	);
};

export default Home;
