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
    gap: 0.5rem;
    padding: 1rem;
    border: 0.1rem solid black;
    border-radius: 0.2rem;
    width: 16rem;
  }
</style>