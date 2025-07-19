# Y11

This project is pretty much still in the brainstorming phase, where I try out random things.
Don't expect to find anything useful here yet.

This project is split into several components, each has it's own branch.
Check them out using `make init`, it'll create a worktree for each of them.  
Now, why not submodules? They are a real pain to work with. They point to a specific commit,
which needs updating and so on. We don't need that. Usually, we'll work on the latest version of all repos.
An option to work on earlier versions may be added later to the init script, but if so, we may do it by time
and / or correlate tag versions for the components.  
You may also wonder, why not just one folder for each component? Because they are somewhat independent,
and will be worked on somewhat independently. The flexibility will be needed. Also, if other projects
use, for example, libY11-server, they won't be developed in lockstep with it either.
