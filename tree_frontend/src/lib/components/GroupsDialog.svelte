<script lang="ts">
  import { groups } from "$lib/state/groups";
  import GroupItem from "./GroupItem.svelte";
</script>

<div id="dialog">
  {#each groups.keys() as group_name}
    <GroupItem {group_name} />
  {/each}
</div>

<style>
  #dialog {
    display: flex;
    flex-direction: column;
    /* gap: 1rem; */
    padding: 1.5rem;
    border-radius: 0.2rem;
    width: 16rem;
    border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    background: white;
  }
</style>