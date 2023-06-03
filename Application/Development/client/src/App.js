import React from 'react';
import './App.css';
//import Navbar from './components/Navbar/NavbarElements';
//import { Navbar } from './components/Navbar';
import Navbar from './components/Navbar';
import { BrowserRouter as Router, Routes, Route, Navigate, Link } from 'react-router-dom';
import Home from './pages/Home';
import Replay from './pages/Replay';
import Connected from './pages/Connected';

function App() {
	return (
			<Router>
				<Routes>
					<Route exact path="/" element={<Home />} />
					<Route path="/connected" element={<Connected />} />
					<Route path="/replay" element={<Replay />} />
					<Route path="/" element={<Navigate replace to="/home" />} /> {/* TODO: add error alert*/}
				</Routes>
      		</Router>
	);
}

export default App;
