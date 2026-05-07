ToDo:
-Re-Refactor the code, into a less horrendous file-splitting.
-Add Torch texture to the Repo.

Refactor render_texture and render_balls into one function, that first uses
the ball->color attr to determine the mode, if color = {0,0,0,0}, switch to texture mode
insert the ball->texture attr into SDL_RenderTexture

->change all occurences of render_texture / render_balls to this fn, change the corrosponding 
	spawn_ball call, to include the texture

Random Movement for the colonists
 -> Later pathfinding to shops / workhouses

Add collision between troops, colonists and the map
	-> it is enough to have a circular hitbox for troop-troop collision
