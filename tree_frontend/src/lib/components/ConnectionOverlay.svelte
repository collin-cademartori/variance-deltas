<script lang="ts">
  import { connection } from "$lib/websocket.svelte";
</script>

{#if !connection.connected}
  <div class="status-bar">
    <span class="title">Connection Lost</span>
    <span class="detail">The backend server has disconnected. Please restart the application.</span>
  </div>
{/if}

<style>
  .status-bar {
    background-color: rgb(255, 240, 240);
    border: 0.1rem solid rgb(200, 100, 100);
    border-radius: 0.2rem;
    padding: 0.5rem 0.75rem;
    display: flex;
    flex-direction: column;
    gap: 0.25rem;
  }

  .title {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
    font-weight: 600;
    color: rgb(150, 50, 50);
  }

  .detail {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    color: rgb(100, 60, 60);
  }
</style>
