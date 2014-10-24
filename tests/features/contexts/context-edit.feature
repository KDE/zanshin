Feature: Context rename
  As someone collecting tasks
  I can rename a context
  In order to refine my tasks organization

  Scenario: Renamed context appear in the list
    Given I display the available pages
    When I rename a "context" named "Errands" to "Chores"
    And I list the items
    Then the list is:
       | display                           | icon              |
       | Contexts                          | folder            |
       | Contexts / Chores                 | view-pim-tasks    |
       | Contexts / Internet               | view-pim-tasks    |
       | Contexts / Online                 | view-pim-tasks    |
       | Inbox                             | mail-folder-inbox |
       | Projects                          | folder            |
       | Projects / Read List              | view-pim-tasks    |
       | Projects / Backlog                | view-pim-tasks    |
       | Projects / Prepare talk about TDD | view-pim-tasks    |
       | Tags                              | folder            |
       | Tags / Philosophy                 | view-pim-tasks    |
       | Tags / Physics                    | view-pim-tasks    |
