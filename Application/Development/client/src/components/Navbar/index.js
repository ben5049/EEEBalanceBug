import React from "react";
import { Nav, NavLink, NavMenu }
    from "./NavbarElements";
 
const Navbar = () => {
    return (
        <>
            <Nav>
                <NavMenu>
                    <NavLink to="/finish" activeStyle>
                        Complete Map
                    </NavLink>
                    <NavLink to="/connected" activeStyle>
                        Connected
                    </NavLink>
                    <NavLink to="/replay" activeStyle>
                        Replay
                    </NavLink>
                </NavMenu>
            </Nav>
        </>
    );
};
 
export default Navbar;