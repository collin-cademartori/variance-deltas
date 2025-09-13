import { clear_branches, clear_selection } from "./selection.svelte.ts";
import { reset_styles } from "./draw_tree.ts";

type user_state_t = 'base' | 'extruding' | 'dividing' | 'auto-dividing' | 'merging' | 'auto-merging';
let _user_state : user_state_t = $state('base');

export const user_state : { state: user_state_t } = $state({ 
  get state() {
    return(_user_state);
  },
  set state(st: user_state_t) {
    clear_selection();
    clear_branches();
    reset_styles();
    _user_state = st;
  }
});
