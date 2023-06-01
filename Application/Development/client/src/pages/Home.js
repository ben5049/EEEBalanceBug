import React from 'react';
import RoverCarousel from '../components/Home/RoverCarousel';
import ReplayCarousel from '../components/Home/ReplayCarousel';
import '../components/Home/grid_Home.css';
import '../components/grid.css';

const Home = () => {
	// Gets from server: MAC, Nickname, Connection status
	const rovers_list = [
		{MAC: '11:11:11:11:11:11', nickname: 'David2', overlayText: 'CONNECTED' },
		{MAC: '13:13:13:13:13:13', nickname: 'Kyle', overlayText: 'OFFLINE' },
		{MAC: '00:00:00:00:00:00', nickname: 'David', overlayText: 'OFFLINE' },
	  ];

	return (
		<div>
			<div class="wrapper">
				<div class="box a_Home">A</div>
				<div class="box b_Home">B</div>
				<div class="box c_Home">C</div>
				<div class="box d_Home">D</div>
				<div class="box e_Home">E</div>
			</div>
			{/*
			<h1>Rovers</h1>
			<RoverCarousel rovers={rovers_list}/>
			<h1>Replays</h1>
			<ReplayCarousel />
			*/}
		</div>
	);
};

export default Home;
